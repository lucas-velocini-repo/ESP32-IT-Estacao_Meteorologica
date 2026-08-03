#pragma once

struct SensorData
{
    // AHT20 + BMP280
    float temperature = 0.0f;
    float humidity = 0.0f;
    float pressure = 0.0f;

    // BH1750
    float light = 0.0f;

    // SPS30 - Massa de partículas
    float pm1_0 = 0.0f;
    float pm2_5 = 0.0f;
    float pm4_0 = 0.0f;
    float pm10_0 = 0.0f;

    // SPS30 - Número de partículas
    float nc0_5 = 0.0f;
    float nc1_0 = 0.0f;
    float nc2_5 = 0.0f;
    float nc4_0 = 0.0f;
    float nc10_0 = 0.0f;

    // SPS30
    float typicalSize = 0.0f;
};