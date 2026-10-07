#pragma once

#include "sensor-data.h"

#include <Adafruit_AHTX0.h>
#include <Adafruit_BMP280.h>

class AHT20BMP280
{
public:
    void begin();

    void update(
        SensorData& data
    );

private:
    Adafruit_AHTX0 aht;
    Adafruit_BMP280 bmp;

    bool ahtReady = false;
    bool bmpReady = false;

    float temperature = 0.0f;
    float humidity = 0.0f;
    float pressure = 0.0f;
};