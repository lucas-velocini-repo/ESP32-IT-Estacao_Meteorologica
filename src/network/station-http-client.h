#pragma once

#include <Arduino.h>
#include <string>

#include "config/settings-manager.h"

class StationHttpClient
{
public:
    void begin(
        SettingsManager& settingsManager
    );

    bool send(
        const std::string& payload
    );
    bool sendLocation(const std::string& payload);

private:
    bool request(const std::string& payload, const char* method, bool location);
    SettingsManager* settings = nullptr;
};
