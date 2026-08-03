#include "bh1750.h"

#include <Arduino.h>

void BH1750Sensor::begin()
{
    light = random(0, 10000) / 10.0f;
}

void BH1750Sensor::update(
    SensorData& data
)
{
    light += random(-50, 51) / 10.0f;

    if(light < 0.0f)
        light = 0.0f;

    if(light > 1000.0f)
        light = 1000.0f;

    data.light = light;
}