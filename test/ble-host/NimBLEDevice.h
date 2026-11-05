#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <utility>
#include <functional>
constexpr uint16_t BLE_HS_CONN_HANDLE_NONE = 0xffff;
namespace NIMBLE_PROPERTY { constexpr int WRITE = 1, NOTIFY = 2; }
class NimBLECharacteristic;
struct NimBLEServer;
struct NimBLEConnInfo {
    uint16_t handle;
    uint16_t getConnHandle() const { return handle; }
};
struct NimBLECharacteristicCallbacks {
    virtual ~NimBLECharacteristicCallbacks() = default;
    virtual void onWrite(NimBLECharacteristic*, NimBLEConnInfo&) {}
    virtual void onSubscribe(NimBLECharacteristic*, NimBLEConnInfo&, uint16_t) {}
};
struct NimBLEServerCallbacks {
    virtual ~NimBLEServerCallbacks() = default;
    virtual void onConnect(NimBLEServer*, NimBLEConnInfo&) {}
    virtual void onDisconnect(NimBLEServer*, NimBLEConnInfo&, int) {}
};
struct FakeNotification { std::string bytes; uint16_t handle; };
extern std::vector<FakeNotification> packets;
extern unsigned notifyFailures;
extern unsigned deferredNotifications;
extern uint16_t peerMtu;
extern std::function<void()> afterPacket;
class NimBLECharacteristic {
public:
    NimBLECharacteristicCallbacks* callbacks = nullptr;
    std::string value;
    void setCallbacks(NimBLECharacteristicCallbacks* cb) { callbacks = cb; }
    std::string getValue() const { return value; }
    void setValue(const std::string& next) { value = next; }
    // Model the bare notify overload: the BLE host reads the mutable value later.
    bool notify() { ++deferredNotifications; return true; }
    bool notify(const uint8_t* data, size_t size, uint16_t handle) {
        if (notifyFailures) { --notifyFailures; return false; }
        packets.push_back({std::string(reinterpret_cast<const char*>(data), size), handle});
        if (afterPacket) {
            auto callback = std::move(afterPacket);
            afterPacket = nullptr;
            callback();
        }
        return true;
    }
};
struct NimBLEService {
    NimBLECharacteristic rx, tx;
    NimBLECharacteristic* createCharacteristic(const char*, int property) { return property == NIMBLE_PROPERTY::WRITE ? &rx : &tx; }
    void start() {}
};
struct NimBLEAdvertising {
    void addServiceUUID(const char*) {}
    void enableScanResponse(bool) {}
    void setName(const char*) {}
    void start() {}
};
struct NimBLEServer {
    NimBLEServerCallbacks* callbacks = nullptr;
    NimBLEService service;
    void setCallbacks(NimBLEServerCallbacks* cb) { callbacks = cb; }
    NimBLEService* createService(const char*) { return &service; }
    std::vector<uint16_t> getPeerDevices() const { return {1}; }
    uint16_t getPeerMTU(uint16_t) const { return peerMtu; }
};
extern NimBLEServer fakeServer;
extern NimBLEAdvertising fakeAdvertising;
struct NimBLEDevice {
    static void init(const char*) {}
    static void setMTU(unsigned) {}
    static NimBLEServer* createServer() { return &fakeServer; }
    static NimBLEAdvertising* getAdvertising() { return &fakeAdvertising; }
    static void startAdvertising() {}
};
