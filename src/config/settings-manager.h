#pragma once

#include <Arduino.h>
#include <Preferences.h>

class SettingsManager
{
public:
    void begin();

    void saveWifi(
        const String& ssid,
        const String& password,
        const String& server
    );

    String getSSID();

    String getPassword();

    String getServer();

private:
    Preferences preferences;
};