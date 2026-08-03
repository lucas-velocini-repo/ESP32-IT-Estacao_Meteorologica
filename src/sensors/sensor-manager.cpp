#include "sensor-manager.h"

void SensorManager::begin()
{
    climate.begin();

    light.begin();

    particulate.begin();
}

SensorData SensorManager::read()
{
    SensorData data;

    climate.update(data);

    light.update(data);

    particulate.update(data);

    return data;
}