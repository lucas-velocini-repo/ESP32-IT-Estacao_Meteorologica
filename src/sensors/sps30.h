#pragma once

#include "sensor-data.h"

#include <SensirionI2cSps30.h>

class SPS30
{
public:
    void begin();

    void update(
        SensorData& data
    );

private:
    SensirionI2cSps30 sensor;

    bool sensorReady = false;

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

    void copyToData(
        SensorData& data
    ) const;
};