#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include <mutex>
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
    bool saveMeasurementConfiguration(uint32_t seconds, uint32_t revision, bool bleOverride = false);
    bool hasBleIntervalOverride();
    void confirmBleInterval(uint32_t seconds, uint32_t revision);
    bool applyRemoteMeasurementConfiguration(uint32_t seconds, uint32_t revision);
    struct MeasurementConfiguration { uint32_t seconds; uint32_t revision; bool bleOverride; };
    MeasurementConfiguration getMeasurementConfiguration();
    String getStationName();
    void saveStationName(const String& name);

    String getDeviceId();

    String getApiToken();

    String getSSID();

    String getPassword();

    String getServer();

private:
    Preferences preferences;
    uint32_t measurementIntervalSeconds = 300;
    uint32_t configurationRevision = 0;
    bool bleIntervalOverride = false;
    std::recursive_mutex intervalMutex;
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
