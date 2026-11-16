#pragma once

#include "sensor-data.h"
#include "aht20-bmp280.h"
#include "bh1750.h"
#include "sps30.h"

class SensorManager
{
public:
    void begin(int sdaPin, int sclPin, bool batteryMode = false);
    SensorData read();
    void setBatteryMode(bool enabled);
    bool prepareBatteryMeasurement();
    void prepareBatteryLight();
    bool readBattery(SensorData& data, bool allowMissingParticulate);
    void suspendBatteryMeasurement();

private:
    bool batteryMode = false;
    AHT20BMP280 climate;
    BH1750Sensor light;
    SPS30 particulate;
};
