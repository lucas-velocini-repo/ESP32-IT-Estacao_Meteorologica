#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <string>

#include "bluetooth/ble-manager.h"
#include "network/wifi-manager.h"
#include "config/settings-manager.h"

class ProtocolHandler
{
private:
    BLEManager* ble = nullptr;
    WiFiManager* wifi = nullptr;
    SettingsManager* settings = nullptr;

    void handleGetStatus();

    void handleConfigureWifi(
        JsonDocument& doc
    );

    struct PendingWifiConfig
    {
        bool pending = false;

        String ssid;
        String password;
        String server;
    };

    PendingWifiConfig pendingWifiConfig;

public:
    void begin(
        BLEManager& ble,
        WiFiManager& wifi,
        SettingsManager& settings
    );

    void handle(
        const std::string& message
    );

    bool hasPendingWifiConfiguration() const;

    PendingWifiConfig takePendingWifiConfiguration();
};