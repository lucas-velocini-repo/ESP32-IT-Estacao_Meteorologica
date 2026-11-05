#pragma once

#include <Arduino.h>
#include <NimBLEDevice.h>
#include <functional>
#include <mutex>
#include <deque>
#include <utility>

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
    friend class NotificationCallbacks;

    void handleConnect(uint16_t handle);
    void handleDisconnect(uint16_t handle);
    void handleSubscribe(uint16_t handle, uint16_t subscription);
    void handleMessage(const std::string& message);

    NimBLEServer* server = nullptr;
    NimBLEService* service = nullptr;
    NimBLECharacteristic* rx = nullptr;
    NimBLECharacteristic* tx = nullptr;

    bool connected = false;
    bool notificationsEnabled = false;
    uint16_t connectionHandle = BLE_HS_CONN_HANDLE_NONE;
    std::string incoming;
    unsigned long lastFragmentAt = 0;
    mutable std::mutex transmitMutex;
    struct PendingResponse {
        uint32_t id;
        std::string frame;
        size_t offset = 0;
        uint8_t failures = 0;
        PendingResponse(uint32_t id, std::string value) : id(id), frame(std::move(value)) {}
    };
    std::deque<PendingResponse> responses;
    uint32_t nextResponseId = 0;

    MessageCallback messageCallback;
};
