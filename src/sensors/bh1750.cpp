#include "bh1750.h"

#include <Arduino.h>
#include <Wire.h>
#include <math.h>

void BH1750Sensor::begin()
{
    Serial.println();
    Serial.println(
        "[BH1750] Inicializando sensor..."
    );

    constexpr uint8_t BH1750_ADDRESS =
        0x23;

    sensorReady =
        sensor.begin(
            BH1750::CONTINUOUS_HIGH_RES_MODE,
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