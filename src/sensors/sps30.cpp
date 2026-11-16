#include "sps30.h"

#include <Arduino.h>
#include <Wire.h>

void SPS30::begin(bool batteryMode)
{
    sensor.begin(Wire, SPS30_I2C_ADDR_69);
    bound = true;
    // O ESP pode ter reiniciado sem reiniciar a alimentação do sensor.
    sensor.wakeUpSequence();
    sensor.stopMeasurement();
    delay(20);
    measuring = false;
    sleeping = false;
    if (batteryMode) suspend();
    else start();
}

bool SPS30::start()
{
    if (!bound) return false;
    latest = SensorData(); // Valores PM nulos até uma leitura nova, inclusive em erro.
    if (measuring) return true;
    if (sleeping)
    {
        const int16_t error = sensor.wakeUpSequence();
        if (error)
        {
            Serial.printf("[SPS30] Falha ao acordar: %d\n", error);
            return false;
        }
        sleeping = false;
    }
    const int16_t error = sensor.startMeasurement(SPS30_OUTPUT_FORMAT_OUTPUT_FORMAT_FLOAT);
    if (error)
    {
        Serial.printf("[SPS30] Falha ao iniciar: %d\n", error);
        return false;
    }
    measuring = true;
    return true;
}

bool SPS30::suspend()
{
    if (!bound || sleeping) return sleeping;
    if (measuring)
    {
        const int16_t error = sensor.stopMeasurement();
        if (error)
        {
            Serial.printf("[SPS30] Falha ao parar: %d\n", error);
            return false;
        }
        measuring = false;
    }
    const int16_t error = sensor.sleep();
    if (error)
    {
        // Firmware antigo pode não ter sleep: permanece em Idle (fan/laser OFF).
        Serial.printf("[SPS30] Sleep indisponível (%d); mantido em Idle.\n", error);
        return false;
    }
    sleeping = true;
    return true;
}

bool SPS30::update(SensorData& data)
{
    if (!measuring) return false;
    uint16_t ready = 0;
    if (sensor.readDataReadyFlag(ready) || !ready) return false;
    SensorData next;
    const int16_t error = sensor.readMeasurementValuesFloat(
        next.pm1_0, next.pm2_5, next.pm4_0, next.pm10_0,
        next.nc0_5, next.nc1_0, next.nc2_5, next.nc4_0, next.nc10_0, next.typicalSize);
    if (error)
    {
        Serial.printf("[SPS30] Falha na leitura: %d\n", error);
        return false;
    }
    latest = next;
    data.pm1_0 = latest.pm1_0;
    data.pm2_5 = latest.pm2_5;
    data.pm4_0 = latest.pm4_0;
    data.pm10_0 = latest.pm10_0;
    data.nc0_5 = latest.nc0_5;
    data.nc1_0 = latest.nc1_0;
    data.nc2_5 = latest.nc2_5;
    data.nc4_0 = latest.nc4_0;
    data.nc10_0 = latest.nc10_0;
    data.typicalSize = latest.typicalSize;
    Serial.printf("[SPS30] PM1=%.2f PM2.5=%.2f PM4=%.2f PM10=%.2f ug/m3\n",
        data.pm1_0, data.pm2_5, data.pm4_0, data.pm10_0);
    return true;
}
