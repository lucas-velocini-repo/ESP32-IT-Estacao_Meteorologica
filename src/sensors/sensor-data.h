#pragma once

#include <cmath>

struct SensorData
{
    // AHT20 + BMP280
    float temperature = NAN;
    float humidity = NAN;
    float pressure = NAN;

    // BH1750
    float light = NAN;

    // SPS30 - Massa de partículas
    float pm1_0 = NAN;
    float pm2_5 = NAN;
    float pm4_0 = NAN;
    float pm10_0 = NAN;

    // SPS30 - Número de partículas
    float nc0_5 = NAN;
    float nc1_0 = NAN;
    float nc2_5 = NAN;
    float nc4_0 = NAN;
    float nc10_0 = NAN;

    // SPS30
    float typicalSize = NAN;
};