#pragma once

#include "sensor-data.h"

#include <BH1750.h>

class BH1750Sensor
{
public:
    void begin();

    void update(
        SensorData& data
    );

private:
    BH1750 sensor;

    bool sensorReady = false;

    float light = 0.0f;
};