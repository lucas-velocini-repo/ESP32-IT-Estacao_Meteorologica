#pragma once

#include <Arduino.h>
#include <cstdint>

#include "sensors/sensor-data.h"

struct StationData
{
    int deviceId = 0;

    String deviceName;

    int64_t measuredAt = 0;

    double latitude = 0.0;
    double longitude = 0.0;

    SensorData sensors;
};