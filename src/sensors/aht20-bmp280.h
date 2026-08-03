#pragma once

#include "sensor-data.h"

class AHT20BMP280
{
public:
    void begin();

    void update(
        SensorData& data
    );

private:
    float temperature = 25.0f;
    float humidity = 60.0f;
    float pressure = 1013.0f;
};