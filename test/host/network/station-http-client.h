#pragma once
#include "Arduino.h"
struct SettingsManager {
    String id = "CASA-000001", token = "test-token";
    String getDeviceId() const { return id; }
    String getApiToken() const { return token; }
};
struct StationHttpClient {
    bool success = false;
    int calls = 0;
    std::string lastPayload;
    bool sendLocation(const std::string& payload) {
        ++calls;
        lastPayload = payload;
        return success;
    }
};
