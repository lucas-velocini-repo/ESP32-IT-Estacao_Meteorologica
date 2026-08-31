#pragma once

#include "sensor-data.h"

#include "aht20-bmp280.h"
#include "bh1750.h"
#include "sps30.h"

class SensorManager
{
public:

    // Os pinos do I2C são definidos pela main e passados aqui.
    void begin(
        int sdaPin,
        int sclPin
    );

    SensorData read();

private:

    AHT20BMP280 climate;
    BH1750Sensor light;
    SPS30 particulate;
};
