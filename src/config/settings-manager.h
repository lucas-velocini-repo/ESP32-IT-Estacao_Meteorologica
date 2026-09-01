#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include "sensors/sensor-data.h"

class SettingsManager
{
public:
    void begin();

    void saveWifi(
        const String& ssid,
        const String& password,
        const String& server
    );

    void saveDeviceId(const String& deviceId);

    String getDeviceId();

    String getSSID();

    String getPassword();

    String getServer();

private:
    Preferences preferences;
};

class HttpClient
{
public:

    void begin(
        SettingsManager& settings
    );

    bool send(
        const SensorData& data
    );

private:

    SettingsManager* settings = nullptr;
};