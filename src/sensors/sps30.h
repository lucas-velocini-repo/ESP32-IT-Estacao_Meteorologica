#pragma once

#include "sensor-data.h"
#include <SensirionI2cSps30.h>

class SPS30
{
public:
    void begin(bool batteryMode = false);
    bool start();
    bool suspend();
    bool update(SensorData& data);

private:
    SensirionI2cSps30 sensor;
    bool measuring = false;
    bool sleeping = false;
    bool bound = false;
    SensorData latest;
};
