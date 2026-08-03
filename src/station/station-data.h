#pragma once

#include <Arduino.h>

#include "sensors/sensor-data.h"

struct StationData
{
    int deviceId = 0;
    String deviceName;

    uint32_t timestamp = 0;

    double latitude = 0.0;
    double longitude = 0.0;

    SensorData sensors;
};