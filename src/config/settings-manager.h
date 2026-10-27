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

    void saveServer(
        const String& server
    );

    void saveDeviceId(const String& deviceId);

    void saveApiToken(const String& apiToken);

    uint32_t getMeasurementIntervalSeconds();
    uint32_t getConfigurationRevision();
    bool saveMeasurementConfiguration(uint32_t seconds, uint32_t revision);

    String getDeviceId();

    String getApiToken();

    String getSSID();

    String getPassword();

    String getServer();

private:
    Preferences preferences;
    uint32_t measurementIntervalSeconds = 300;
    uint32_t configurationRevision = 0;
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