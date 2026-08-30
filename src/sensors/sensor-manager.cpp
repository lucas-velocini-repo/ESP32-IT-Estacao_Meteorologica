#include "sensor-manager.h"

#include <Arduino.h>
#include <Wire.h>

void scanI2CBus()
{
    Serial.println();
    Serial.println(
        "[I2C] Procurando dispositivos..."
    );

    int devicesFound = 0;

    for(
        uint8_t address = 1;
        address < 127;
        address++
    )
    {
        Wire.beginTransmission(
            address
        );

        uint8_t error =
            Wire.endTransmission();

        if(error == 0)
        {
            Serial.print(
                "[I2C] Encontrado: 0x"
            );

            if(address < 16)
            {
                Serial.print("0");
            }

            Serial.println(
                address,
                HEX
            );

            devicesFound++;
        }
    }

    Serial.print(
        "[I2C] Total encontrado: "
    );

    Serial.println(
        devicesFound
    );
}

namespace
{
    constexpr int I2C_SDA_PIN = 5;
    constexpr int I2C_SCL_PIN = 4;
}

void SensorManager::begin()
{
    Serial.println();
    Serial.println("[Sensors] Inicializando barramento I2C...");

    Wire.setPins(
        I2C_SDA_PIN,
        I2C_SCL_PIN
    );

    Wire.begin();

    Wire.setClock(100000);

    Serial.println("[Sensors] I2C iniciado: SDA=GPIO5, SCL=GPIO4");

    scanI2CBus();

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

