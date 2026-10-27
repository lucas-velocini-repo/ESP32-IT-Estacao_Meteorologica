#include "settings-manager.h"
#include "config.h"
#include <ArduinoJson.h>

void SettingsManager::begin()
{
    preferences.begin("config", false);
    JsonDocument doc;
    deserializeJson(doc, preferences.getString("interval_cfg", "{}"));
    const uint32_t seconds = doc["seconds"] | (DATA_SEND_INTERVAL_MS / 1000);
    measurementIntervalSeconds = seconds >= MIN_MEASUREMENT_INTERVAL_SECONDS && seconds <= MAX_MEASUREMENT_INTERVAL_SECONDS ? seconds : DATA_SEND_INTERVAL_MS / 1000;
    configurationRevision = doc["revision"] | 0U;
}

void SettingsManager::saveWifi(
    const String& ssid,
    const String& password,
    const String& server
)
{
    preferences.putString("ssid", ssid);
    preferences.putString("pass", password);
    preferences.putString("server", server);
}

void SettingsManager::saveServer(
    const String& server
)
{
    preferences.putString(
        "server",
        server
    );
}

void SettingsManager::saveDeviceId(
    const String& deviceId
)
{
    preferences.putString(
        "device_id",
        deviceId
    );
}

void SettingsManager::saveApiToken(
    const String& apiToken
)
{
    preferences.putString(
        "api_token",
        apiToken
    );
}

String SettingsManager::getDeviceId()
{
    if (!preferences.isKey("device_id")) return "";
    return preferences.getString("device_id", "");
}

String SettingsManager::getApiToken()
{
    if (!preferences.isKey("api_token")) return "";
    return preferences.getString("api_token", "");
}

String SettingsManager::getSSID()
{
    if (!preferences.isKey("ssid")) return "";
    return preferences.getString("ssid", "");
}

String SettingsManager::getPassword()
{
    if (!preferences.isKey("pass")) return "";
    return preferences.getString("pass", "");
}

String SettingsManager::getServer()
{
    if (!preferences.isKey("server")) return "";
    return preferences.getString("server", "");
}

uint32_t SettingsManager::getMeasurementIntervalSeconds()
{
    return measurementIntervalSeconds;
}

uint32_t SettingsManager::getConfigurationRevision()
{
    return configurationRevision;
}

bool SettingsManager::saveMeasurementConfiguration(uint32_t seconds, uint32_t revision)
{
    if (seconds < MIN_MEASUREMENT_INTERVAL_SECONDS || seconds > MAX_MEASUREMENT_INTERVAL_SECONDS)
        return false;
    if (seconds == getMeasurementIntervalSeconds() && revision == getConfigurationRevision())
        return true; // Do not wear NVS with identical reports every minute.
    JsonDocument doc;
    doc["seconds"] = seconds;
    doc["revision"] = revision;
    String value;
    serializeJson(doc, value);
    // One NVS value keeps interval and revision together across a power loss.
    if (preferences.putString("interval_cfg", value) != value.length()) return false;
    measurementIntervalSeconds = seconds;
    configurationRevision = revision;
    return true;
}
