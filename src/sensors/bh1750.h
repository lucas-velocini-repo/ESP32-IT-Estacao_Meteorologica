#pragma once

#include "sensor-data.h"

class BH1750Sensor
{
public:
    void begin();

    void update(
        SensorData& data
    );

private:
    float light = 350.0f;
};