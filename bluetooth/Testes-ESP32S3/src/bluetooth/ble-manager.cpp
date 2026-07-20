#include "ble-manager.h"
#include "ble-uuid.h"
#include "../config/config.h"
#include "ble-callbacks.h"

void BLEManager::begin()
{
    Serial.println("Inicializando BLE...");

    NimBLEDevice::init(DEVICE_NAME);

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

    advertising->setName(DEVICE_NAME);

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

    tx->notify();
}