#include "sps30.h"

#include <Arduino.h>

void SPS30::begin()
{
    pm1 = random(80, 140) / 10.0f;

    pm2_5 = pm1 + random(10, 40) / 10.0f;

    pm4 = pm2_5 + random(5, 30) / 10.0f;

    pm10 = pm4 + random(5, 40) / 10.0f;

    nc0_5 = random(8000, 12000) / 10.0f;

    nc1 = nc0_5 * 0.82f;
    nc2_5 = nc0_5 * 0.55f;
    nc4 = nc0_5 * 0.34f;
    nc10 = nc0_5 * 0.15f;

    typicalSize = random(5, 25) / 10.0f;
}

void SPS30::update(
    SensorData& data
)
{
    pm1 += random(-2, 3) / 10.0f;

    if(pm1 < 1.0f)
        pm1 = 1.0f;

    if(pm1 > 25.0f)
        pm1 = 25.0f;

    pm2_5 = pm1 + random(10, 40) / 10.0f;

    pm4 = pm2_5 + random(5, 20) / 10.0f;

    pm10 = pm4 + random(5, 30) / 10.0f;

    nc0_5 += random(-200, 201);

    if(nc0_5 < 300)
        nc0_5 = 300;

    if(nc0_5 > 1500)
        nc0_5 = 1500;

    nc1 = nc0_5 * 0.82f;
    nc2_5 = nc0_5 * 0.55f;
    nc4 = nc0_5 * 0.34f;
    nc10 = nc0_5 * 0.15f;

    typicalSize += random(-1, 2) / 10.0f;

    if(typicalSize < 0.5f)
        typicalSize = 0.5f;

    if(typicalSize > 2.5f)
        typicalSize = 2.5f;

    data.pm1_0 = pm1;
    data.pm2_5 = pm2_5;
    data.pm4_0 = pm4;
    data.pm10_0 = pm10;

    data.nc0_5 = nc0_5;
    data.nc1_0 = nc1;
    data.nc2_5 = nc2_5;
    data.nc4_0 = nc4;
    data.nc10_0 = nc10;

    data.typicalSize = typicalSize;
}