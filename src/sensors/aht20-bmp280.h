#pragma once

#include "sensor-data.h"

#include <Adafruit_AHTX0.h>
#include <Adafruit_BMP280.h>

class AHT20BMP280
{
public:
    void begin();
    void setBatteryMode(bool enabled);

    void update(
        SensorData& data
    );

private:
    Adafruit_AHTX0 aht;
    Adafruit_BMP280 bmp;

    bool batteryMode = false;
    bool ahtReady = false;
    bool bmpReady = false;

    float temperature = NAN;
    float humidity = NAN;
    float pressure = NAN;
};