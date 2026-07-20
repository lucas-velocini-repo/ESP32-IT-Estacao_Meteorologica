#pragma once

#include <Arduino.h>
#include <NimBLEDevice.h>
#include <functional>

class BLEManager
{
public:
    using MessageCallback =
        std::function<void(const std::string&)>;
    void begin();
    void update();
    bool isConnected() const;
    void send(const std::string& message);
    void setMessageCallback(MessageCallback callback);
private:
    friend class ServerCallbacks;
    friend class CharacteristicCallbacks;

    void handleConnect();
    void handleDisconnect();
    void handleMessage(const std::string& message);

    NimBLEServer* server = nullptr;
    NimBLEService* service = nullptr;
    NimBLECharacteristic* rx = nullptr;
    NimBLECharacteristic* tx = nullptr;

    bool connected = false;

    MessageCallback messageCallback;
};