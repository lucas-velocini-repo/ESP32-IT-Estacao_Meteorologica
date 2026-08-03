#include "aht20-bmp280.h"

#include <Arduino.h>

void AHT20BMP280::begin()
{
    randomSeed(micros());

    temperature = random(180, 330) / 10.0f;
    humidity = random(350, 850) / 10.0f;
    pressure = random(9900, 10350) / 10.0f;
}

void AHT20BMP280::update(
    SensorData& data
)
{
    temperature += random(-5, 6) / 10.0f;
    humidity += random(-10, 11) / 10.0f;
    pressure += random(-3, 4) / 10.0f;

    if (temperature < 18.0f) temperature = 18.0f;
    if (temperature > 33.0f) temperature = 33.0f;

    if (humidity < 35.0f) humidity = 35.0f;
    if (humidity > 85.0f) humidity = 85.0f;

    if (pressure < 990.0f) pressure = 990.0f;
    if (pressure > 1035.0f) pressure = 1035.0f;

    data.temperature = temperature;
    data.humidity = humidity;
    data.pressure = pressure;
}