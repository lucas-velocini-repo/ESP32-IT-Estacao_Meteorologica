#include "ble-manager.h"
#include "ble-uuid.h"
#include "../config/config.h"
#include "ble-callbacks.h"
#include "device/device-identity.h"

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
    if(messageCallback)
    {
        messageCallback(message);
    }
}

void BLEManager::handleConnect()
{
    connected = true;

    Serial.println("BLE conectado.");
}

void BLEManager::handleDisconnect()
{
    connected = false;

    Serial.println("BLE desconectado.");

    NimBLEDevice::startAdvertising();
}

void BLEManager::setMessageCallback(
    MessageCallback callback)
{
    messageCallback = std::move(callback);
}

void BLEManager::update()
{
}

bool BLEManager::isConnected() const
{
    return connected;
}

void BLEManager::send(const std::string& message)
{
    if(!connected)
        return;

    tx->setValue(message);

    Serial.print("Tamanho da mensagem: ");
    Serial.println(message.length());

    Serial.println(message.c_str());

    tx->notify();
}