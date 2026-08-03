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

private:
    SettingsManager* settings = nullptr;
};