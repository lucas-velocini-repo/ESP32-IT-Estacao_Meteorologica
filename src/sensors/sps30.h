#pragma once

#include "sensor-data.h"

class SPS30
{
public:
    void begin();

    void update(
        SensorData& data
    );

private:
    float pm1 = 0.0f;
    float pm2_5 = 0.0f;
    float pm4 = 0.0f;
    float pm10 = 0.0f;

    float nc0_5 = 0.0f;
    float nc1 = 0.0f;
    float nc2_5 = 0.0f;
    float nc4 = 0.0f;
    float nc10 = 0.0f;

    float typicalSize = 0.0f;
};