#include "settings-manager.h"

void SettingsManager::begin()
{
    preferences.begin("config", false);
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
    return preferences.getString(
        "device_id",
        ""
    );
}

String SettingsManager::getApiToken()
{
    return preferences.getString(
        "api_token",
        ""
    );
}

String SettingsManager::getSSID()
{
    return preferences.getString("ssid", "");
}

String SettingsManager::getPassword() 
{
    return preferences.getString("pass", "");
}

String SettingsManager::getServer() 
{
    return preferences.getString("server", "");
}