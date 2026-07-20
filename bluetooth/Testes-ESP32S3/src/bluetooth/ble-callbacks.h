#include "ble-manager.h"

class CharacteristicCallbacks
    : public NimBLECharacteristicCallbacks
{
public:
    CharacteristicCallbacks(BLEManager* ble)
        : ble(ble)
    {
    }

    void onWrite(
        NimBLECharacteristic* characteristic,
        NimBLEConnInfo& connInfo) override
    {
        ble->handleMessage(
            characteristic->getValue());
    }
private:
    BLEManager* ble;
};

class ServerCallbacks
    : public NimBLEServerCallbacks
{
public:
    ServerCallbacks(BLEManager* ble)
        : ble(ble)
    {
    }

    void onConnect(
        NimBLEServer* server,
        NimBLEConnInfo& connInfo) override
    {
        ble->handleConnect();
    }

    void onDisconnect(
        NimBLEServer* server,
        NimBLEConnInfo& connInfo,
        int reason) override
    {
        ble->handleDisconnect();
    }

private:
    BLEManager* ble;
};