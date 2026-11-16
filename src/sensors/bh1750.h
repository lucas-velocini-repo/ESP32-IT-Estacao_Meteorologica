#pragma once

#include "sensor-data.h"

#include <BH1750.h>

class BH1750Sensor
{
public:
    void begin(bool batteryMode = false);
    void setBatteryMode(bool enabled);
    void startOneShot();
    void suspend();

    void update(
        SensorData& data
    );

private:
    BH1750 sensor;

    bool sensorReady = false;

    float light = NAN;
};