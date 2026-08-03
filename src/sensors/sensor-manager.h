#pragma once

#include "sensor-data.h"

#include "aht20-bmp280.h"
#include "bh1750.h"
#include "sps30.h"

class SensorManager
{
public:

    void begin();

    SensorData read();

private:

    AHT20BMP280 climate;
    BH1750Sensor light;
    SPS30 particulate;
};