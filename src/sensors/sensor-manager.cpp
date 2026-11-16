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

void SensorManager::begin(
    int sdaPin,
    int sclPin,
    bool battery
)
{
    Serial.println();
    Serial.println("[Sensors] Inicializando barramento I2C...");

    Wire.setPins(
        sdaPin,
        sclPin
    );

    Wire.begin();

    Wire.setClock(100000);

    Serial.print(
        "[Sensors] I2C iniciado: SDA=GPIO"
    );

    Serial.print(sdaPin);

    Serial.print(
        ", SCL=GPIO"
    );

    Serial.println(sclPin);

    scanI2CBus();

    batteryMode = battery;
    climate.begin();
    climate.setBatteryMode(battery);
    light.begin(battery);
    particulate.begin(battery);
}

SensorData SensorManager::read()
{
    SensorData data;

    climate.update(data);
    light.update(data);
    particulate.update(data);

    return data;
}


void SensorManager::setBatteryMode(bool enabled)
{
    if (batteryMode == enabled) return;
    batteryMode = enabled;
    climate.setBatteryMode(enabled);
    light.setBatteryMode(enabled);
    if (enabled) particulate.suspend();
    else particulate.start();
}

bool SensorManager::prepareBatteryMeasurement()
{
    return particulate.start();
}

void SensorManager::prepareBatteryLight()
{
    light.startOneShot();
}

bool SensorManager::readBattery(SensorData& data, bool allowMissingParticulate)
{
    // Cada ciclo começa com PM inválido. Não recicla valores do ciclo anterior.
    const bool freshParticulate = particulate.update(data);
    if (!freshParticulate && !allowMissingParticulate) return false;
    climate.update(data);
    light.update(data);
    return true;
}

void SensorManager::suspendBatteryMeasurement()
{
    light.suspend();
    particulate.suspend();
}
