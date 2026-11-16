#include "bh1750.h"

#include <Arduino.h>
#include <Wire.h>
#include <math.h>

void BH1750Sensor::begin(bool batteryMode)
{
    Serial.println();
    Serial.println(
        "[BH1750] Inicializando sensor..."
    );

    constexpr uint8_t BH1750_ADDRESS =
        0x23;

    sensorReady =
        sensor.begin(
            batteryMode ? BH1750::ONE_TIME_HIGH_RES_MODE : BH1750::CONTINUOUS_HIGH_RES_MODE,
            BH1750_ADDRESS,
            &Wire
        );

    if(sensorReady)
    {
        Serial.println(
            "[BH1750] Sensor encontrado em 0x23."
        );
    }
    else
    {
        Serial.println(
            "[BH1750] ERRO: sensor não encontrado."
        );
    }
    if (batteryMode) suspend();

}


void BH1750Sensor::update(
    SensorData& data
)
{
    if(!sensorReady)
    {
        Serial.println(
            "[BH1750] Sensor não inicializado."
        );

        data.light =
            light;

        return;
    }

    if(!sensor.measurementReady())
    {
        Serial.println(
            "[BH1750] Medição ainda não disponível."
        );

        data.light =
            light;

        return;
    }

    float newLight =
        sensor.readLightLevel();

    if(
        newLight < 0.0f
        || isnan(newLight)
    )
    {
        Serial.println(
            "[BH1750] ERRO durante leitura."
        );

        data.light =
            light;

        return;
    }

    light =
        newLight;

    data.light =
        light;

    Serial.print(
        "[BH1750] Luminosidade: "
    );

    Serial.print(
        light,
        2
    );

    Serial.println(
        " lx"
    );
}
void BH1750Sensor::suspend()
{
    if (!sensorReady) return;
    // POWER DOWN = 0x00 (não é um Mode público da biblioteca BH1750).
    Wire.beginTransmission(0x23);
    Wire.write(0x00);
    Wire.endTransmission();
}

void BH1750Sensor::setBatteryMode(bool enabled)
{
    if (!sensorReady) return;
    if (enabled) suspend();
    else sensor.configure(BH1750::CONTINUOUS_HIGH_RES_MODE);
}

void BH1750Sensor::startOneShot()
{
    light = NAN;
    if (sensorReady) sensor.configure(BH1750::ONE_TIME_HIGH_RES_MODE);
    // Após a conversão, o BH1750 volta automaticamente ao power down.
}
