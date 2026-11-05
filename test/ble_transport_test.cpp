#include <cassert>
#include <iostream>
#include <ArduinoJson.h>
#include "bluetooth/ble-manager.h"

SerialStub Serial;
unsigned long fakeMillis = 0;
unsigned delayCalls = 0, notifyFailures = 0, deferredNotifications = 0;
uint16_t peerMtu = 247;
std::vector<FakeNotification> packets;
NimBLEServer fakeServer;
NimBLEAdvertising fakeAdvertising;
std::function<void()> afterPacket;

void connect(uint16_t handle = 1) {
    NimBLEConnInfo info{handle};
    fakeServer.callbacks->onConnect(&fakeServer, info);
}
void subscribe(uint16_t handle = 1, uint16_t value = 1) {
    NimBLEConnInfo info{handle};
    fakeServer.service.tx.callbacks->onSubscribe(&fakeServer.service.tx, info, value);
}
void disconnect(uint16_t handle = 1) {
    NimBLEConnInfo info{handle};
    fakeServer.callbacks->onDisconnect(&fakeServer, info, 0);
}
void pump(BLEManager& ble, unsigned times = 80) {
    for (unsigned i = 0; i < times; ++i) { fakeMillis += 10; ble.update(); }
}
std::string received() {
    std::string value;
    for (const auto& packet : packets) value += packet.bytes;
    return value;
}

int main() {
    BLEManager ble;
    ble.begin();
    connect();
    assert(ble.isConnected());
    const std::string status = std::string("{\"type\":\"status\",\"ssid\":\"Estação São José\",\"server\":\"") + std::string(400, 'x') + "\"}";
    // Reproduce an immediate reply inside onWrite, before subscription finishes.
    ble.setMessageCallback([&](const std::string&) { ble.send(status); });
    fakeServer.service.rx.value = "{\"command\":\"get_status\"}";
    NimBLEConnInfo info{1};
    fakeServer.service.rx.callbacks->onWrite(&fakeServer.service.rx, info);
    assert(delayCalls == 0); // No sleeping inside the BLE host callback.
    pump(ble);
    assert(packets.empty());
    subscribe();
    pump(ble);
    assert(packets.size() > 1);
    assert(received() == status + "\n");
    assert(deferredNotifications == 0); // Regression: never read a shared value later.
    for (const auto& packet : packets) assert(packet.bytes.size() <= 180 && packet.handle == 1);

    packets.clear();
    peerMtu = 23;
    notifyFailures = 2;
    ble.send(status);
    pump(ble);
    assert(received() == status + "\n");
    for (const auto& packet : packets) assert(packet.bytes.size() <= 20);

    packets.clear();
    ble.send(status);
    afterPacket = [] { disconnect(); connect(2); subscribe(2); };
    pump(ble, 1);
    assert(packets.size() == 1);
    assert(ble.isConnected());
    packets.clear();
    ble.send("{\"type\":\"fresh\"}");
    pump(ble);
    assert(received() == "{\"type\":\"fresh\"}\n");
    for (const auto& packet : packets) assert(packet.handle == 2);

    packets.clear();
    notifyFailures = 5;
    ble.send("{\"type\":\"failed\"}");
    ble.send("{\"type\":\"next\"}");
    pump(ble);
    assert(received() == "{\"type\":\"next\"}\n");
    disconnect(2);
    assert(!ble.isConnected());
    std::cout << "BLE immutable fragments, subscription race, MTU, retry and reconnect checks passed\n";
}
