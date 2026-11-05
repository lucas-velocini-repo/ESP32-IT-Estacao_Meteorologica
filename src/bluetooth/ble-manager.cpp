#include "ble-manager.h"
#include "ble-uuid.h"
#include "../config/config.h"
#include "ble-callbacks.h"
#include "device/device-identity.h"
#include <ArduinoJson.h>
#include <algorithm>

void BLEManager::begin()
{
    Serial.println("Inicializando BLE...");

    const String hardwareId = DeviceIdentity::getHardwareId();
    const String name = "CASA-" + hardwareId.substring(hardwareId.length() - 6);
    NimBLEDevice::init(name.c_str());
    NimBLEDevice::setMTU(247);

    server = NimBLEDevice::createServer();
    server->setCallbacks(new ServerCallbacks(this));

    service = server->createService(BLEUUIDs::SERVICE);

    rx = service->createCharacteristic(
        BLEUUIDs::RX,
        NIMBLE_PROPERTY::WRITE);
    
    rx->setCallbacks(new CharacteristicCallbacks(this));

    tx = service->createCharacteristic(
        BLEUUIDs::TX,
        NIMBLE_PROPERTY::NOTIFY);
    tx->setCallbacks(new NotificationCallbacks(this));

    service->start();

    NimBLEAdvertising* advertising =
        NimBLEDevice::getAdvertising();

    advertising->addServiceUUID(BLEUUIDs::SERVICE);

    // The 128-bit UART UUID plus flags leaves insufficient room for the name.
    // Put the complete, stable identifier into the active scan response.
    advertising->enableScanResponse(true);
    advertising->setName(name.c_str());

    advertising->start();

    Serial.println("BLE pronto.");
}

void BLEManager::handleMessage(
    const std::string& message)
{
    const unsigned long now = millis();
    if (now - lastFragmentAt > 5000) incoming.clear();
    lastFragmentAt = now;
    incoming += message;
    if (incoming.size() > 4096) { incoming.clear(); return; }
    size_t end;
    while ((end = incoming.find('\n')) != std::string::npos)
    {
        const std::string frame = incoming.substr(0, end);
        incoming.erase(0, end + 1);
        if (!frame.empty() && messageCallback) messageCallback(frame);
    }
    // Accept complete unframed JSON from older app versions as well.
    JsonDocument doc;
    if (!incoming.empty() && !deserializeJson(doc, incoming))
    {
        const std::string frame = incoming;
        incoming.clear();
        if (messageCallback) messageCallback(frame);
    }
}

void BLEManager::handleConnect(uint16_t handle)
{
    std::lock_guard<std::mutex> lock(transmitMutex);
    connected = true;
    connectionHandle = handle;
    notificationsEnabled = false;
    responses.clear();
    incoming.clear();

    Serial.println("BLE conectado.");
}

void BLEManager::handleDisconnect(uint16_t handle)
{
    {
        std::lock_guard<std::mutex> lock(transmitMutex);
        if (handle != connectionHandle) return;
        connected = false;
        connectionHandle = BLE_HS_CONN_HANDLE_NONE;
        notificationsEnabled = false;
        responses.clear();
        incoming.clear();
    }

    Serial.println("BLE desconectado.");

    NimBLEDevice::startAdvertising();
}

void BLEManager::handleSubscribe(uint16_t handle, uint16_t subscription)
{
    std::lock_guard<std::mutex> lock(transmitMutex);
    if (handle != connectionHandle) return;
    notificationsEnabled = (subscription & 1) != 0;
    if (!notificationsEnabled) responses.clear();
    Serial.println(notificationsEnabled
        ? "[BLE] Notificacoes de resposta ativadas."
        : "[BLE] Notificacoes de resposta desativadas.");
}

void BLEManager::setMessageCallback(
    MessageCallback callback)
{
    messageCallback = std::move(callback);
}

void BLEManager::update()
{
    // Complete one queued response before the loop's potentially slow HTTP work.
    // Never wait or hold our mutex across NimBLE calls inside its host callback.
    while (true)
    {
        uint16_t handle;
        uint32_t responseId;
        {
            std::lock_guard<std::mutex> lock(transmitMutex);
            if (!connected || !notificationsEnabled || responses.empty()) return;
            handle = connectionHandle;
            responseId = responses.front().id;
        }
        const uint16_t mtu = server->getPeerMTU(handle);
        const size_t capacity = mtu > 3 ? std::min<size_t>(180, mtu - 3) : 20;
        std::string fragment;
        size_t offset;
        {
            std::lock_guard<std::mutex> lock(transmitMutex);
            if (!connected || !notificationsEnabled || connectionHandle != handle ||
                responses.empty() || responses.front().id != responseId) return;
            offset = responses.front().offset;
            fragment = responses.front().frame.substr(offset, capacity);
        }
        // Explicit bytes get a separate immutable mbuf. Bare notify() instead
        // schedules a later read and can coalesce overwritten fragments.
        const bool sent = tx->notify(reinterpret_cast<const uint8_t*>(fragment.data()), fragment.size(), handle);
        {
            std::lock_guard<std::mutex> lock(transmitMutex);
            if (responses.empty() || responses.front().id != responseId ||
                responses.front().offset != offset) return;
            auto& response = responses.front();
            if (!sent)
            {
                if (++response.failures >= 5)
                {
                    Serial.println("[BLE] Falha ao enviar resposta apos cinco tentativas.");
                    responses.pop_front();
                }
                return;
            }
            response.failures = 0;
            response.offset += fragment.size();
            if (response.offset == response.frame.size())
            {
                Serial.printf("[BLE] Resposta enviada: %u bytes.\n", static_cast<unsigned>(response.frame.size()));
                responses.pop_front();
                return;
            }
        }
        delay(1); // Yield to the BLE host, outside onWrite and outside our mutex.
    }
}

bool BLEManager::isConnected() const
{
    std::lock_guard<std::mutex> lock(transmitMutex);
    return connected;
}

void BLEManager::send(const std::string& message)
{
    JsonDocument doc;
    std::string frame;
    if (!deserializeJson(doc, message)) serializeJson(doc, frame);
    else frame = message;
    frame += '\n';
    std::lock_guard<std::mutex> lock(transmitMutex);
    if (!connected) return;
    if (frame.size() > 4096 || responses.size() >= 8)
    {
        Serial.println("[BLE] Resposta descartada: limite da fila de transmissao.");
        return;
    }
    responses.emplace_back(++nextResponseId, std::move(frame));
}
