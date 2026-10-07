#include <cassert>
#include <cmath>
#include <iostream>
#include <ArduinoJson.h>
#include <WiFi.h>
#include "location/location-manager.h"

SerialStub Serial;
WiFiStub WiFi;
int wifiStatus = 0;
uint64_t uptimeMs = 0;
bool storageFails = false;
std::map<std::string, std::string> nvs;

bool savedPending() {
    JsonDocument doc;
    assert(!deserializeJson(doc, nvs.at("state")));
    return doc["pending"].as<bool>();
}

int main() {
    GNSSManager gps;
    TimeManager clock;
    SettingsManager settings;
    StationHttpClient http;
    LocationManager initial;
    initial.begin();
    initial.update(gps, clock, settings, http);
    assert(!initial.getData().valid);

    // Offline boot with GPS UTC does not require NTP.
    uptimeMs = 1000;
    gps.data.valid = true;
    gps.data.latitude = -23.123456789;
    gps.data.longitude = -47.123456789;
    gps.data.acquiredAt = 1791223200LL;
    gps.data.sequence = 1;
    initial.update(gps, clock, settings, http);
    assert(initial.getData().valid);
    assert(http.calls == 0);
    assert(savedPending());

    // Power cycle restores both position precision and the original timestamp.
    LocationManager reboot;
    uptimeMs = 0;
    reboot.begin();
    assert(reboot.getData().valid);
    assert(std::abs(reboot.getData().latitude - gps.data.latitude) < 1e-8);
    assert(reboot.getData().acquiredAt == 1791223200LL);
    wifiStatus = WL_CONNECTED;
    reboot.update(gps, clock, settings, http);
    assert(http.calls == 1); // failed request leaves pending in NVS
    assert(savedPending());
    JsonDocument sent;
    assert(!deserializeJson(sent, http.lastPayload));
    assert(sent["acquired_at"].as<int64_t>() == 1791223200LL);
    assert(!sent["device_id"].is<String>());
    uptimeMs = 59999;
    reboot.update(gps, clock, settings, http);
    assert(http.calls == 1);
    http.success = true;
    uptimeMs = 60000;
    reboot.update(gps, clock, settings, http);
    assert(http.calls == 2);
    assert(!savedPending());

    // A new boot fix replaces the restored record and resets the 24h interval.
    wifiStatus = 0;
    uptimeMs = 61000;
    gps.data.sequence = 2;
    gps.data.acquiredAt += 61;
    gps.data.latitude = -22;
    reboot.update(gps, clock, settings, http);
    assert(reboot.getData().latitude == -22);
    assert(savedPending());
    const int64_t acquired = gps.data.acquiredAt;
    gps.data.sequence = 3;
    gps.data.latitude = -21;
    gps.data.acquiredAt += 100;
    uptimeMs = 61000 + LocationSchedule::DAILY_MS - 1;
    reboot.update(gps, clock, settings, http);
    assert(reboot.getData().acquiredAt == acquired);
    uptimeMs += 1;
    reboot.update(gps, clock, settings, http);
    assert(reboot.getData().acquiredAt == acquired); // pre-search fix excluded
    ++gps.data.sequence;
    ++gps.data.acquiredAt;
    ++uptimeMs;
    reboot.update(gps, clock, settings, http);
    assert(reboot.getData().latitude == -21);

    // Re-provisioning cannot reuse a position belonging to another station.
    settings.id = "CASA-000002";
    reboot.update(gps, clock, settings, http);
    assert(!reboot.getData().valid);
    gps.data.acquiredAt = 0;
    ++gps.data.sequence;
    reboot.update(gps, clock, settings, http);
    assert(!reboot.getData().valid); // neither GPS UTC nor NTP available
    clock.epoch = 1791309600LL;
    reboot.update(gps, clock, settings, http);
    assert(reboot.getData().valid);
    assert(reboot.getData().acquiredAt == clock.epoch);

    // Failure to persist a newly accepted fix prevents premature HTTP sending.
    settings.id = "CASA-000003";
    reboot.update(gps, clock, settings, http);
    storageFails = true;
    wifiStatus = WL_CONNECTED;
    const int before = http.calls;
    ++gps.data.sequence;
    ++clock.epoch;
    reboot.update(gps, clock, settings, http);
    assert(http.calls == before);
    storageFails = false;
    uptimeMs += 60000;
    reboot.update(gps, clock, settings, http);
    assert(http.calls == before + 1);
    assert(!savedPending());
    std::cout << "Location persistence, offline retries, identity and clock checks passed\n";
}
