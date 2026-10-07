#include "sps30.h"

#include <Arduino.h>
#include <Wire.h>

void SPS30::begin()
{
    Serial.println();
    Serial.println(
        "[SPS30] Inicializando sensor..."
    );

    // O Wire já foi inicializado pelo SensorManager.
    sensor.begin(
        Wire,
        SPS30_I2C_ADDR_69
    );

    // Garante que o sensor esteja em Idle Mode
    // antes de iniciar uma nova medição.
    int16_t error =
        sensor.stopMeasurement();

    if(error != 0)
    {
        Serial.print(
            "[SPS30] Aviso ao parar medição anterior. Código: "
        );

        Serial.println(error);
    }

    delay(100);

    // Inicia medição contínua usando valores float.
    error =
        sensor.startMeasurement(
            SPS30_OUTPUT_FORMAT_OUTPUT_FORMAT_FLOAT
        );

    if(error != 0)
    {
        Serial.print(
            "[SPS30] ERRO ao iniciar medição. Código: "
        );

        Serial.println(error);

        sensorReady = false;

        return;
    }

    sensorReady = true;

    Serial.println(
        "[SPS30] Medição iniciada com sucesso."
    );
}


void SPS30::update(
    SensorData& data
)
{
    if(!sensorReady)
    {
        Serial.println(
            "[SPS30] Sensor não inicializado."
        );

        copyToData(data);

        return;
    }

    // -----------------------------
    // Verifica se existe dado novo
    // -----------------------------

    uint16_t dataReadyFlag = 0;

    int16_t error =
        sensor.readDataReadyFlag(
            dataReadyFlag
        );

    if(error != 0)
    {
        Serial.print(
            "[SPS30] ERRO ao verificar dados. Código: "
        );

        Serial.println(error);

        copyToData(data);

        return;
    }

    if(dataReadyFlag == 0)
    {
        Serial.println(
            "[SPS30] Nova medição ainda não disponível."
        );

        copyToData(data);

        return;
    }

    // -----------------------------
    // Variáveis temporárias
    // -----------------------------

    float newPm1 = 0.0f;
    float newPm2_5 = 0.0f;
    float newPm4 = 0.0f;
    float newPm10 = 0.0f;

    float newNc0_5 = 0.0f;
    float newNc1 = 0.0f;
    float newNc2_5 = 0.0f;
    float newNc4 = 0.0f;
    float newNc10 = 0.0f;

    float newTypicalSize = 0.0f;

    // -----------------------------
    // Leitura real do SPS30
    // -----------------------------

    error =
        sensor.readMeasurementValuesFloat(
            newPm1,
            newPm2_5,
            newPm4,
            newPm10,

            newNc0_5,
            newNc1,
            newNc2_5,
            newNc4,
            newNc10,

            newTypicalSize
        );

    if(error != 0)
    {
        Serial.print(
            "[SPS30] ERRO durante leitura. Código: "
        );

        Serial.println(error);

        copyToData(data);

        return;
    }

    // -----------------------------
    // Atualiza última leitura válida
    // -----------------------------

    pm1 = newPm1;
    pm2_5 = newPm2_5;
    pm4 = newPm4;
    pm10 = newPm10;

    nc0_5 = newNc0_5;
    nc1 = newNc1;
    nc2_5 = newNc2_5;
    nc4 = newNc4;
    nc10 = newNc10;

    typicalSize =
        newTypicalSize;

    // -----------------------------
    // Log para teste
    // -----------------------------

    Serial.println();
    Serial.println(
        "[SPS30] Leitura realizada:"
    );

    Serial.print("PM1.0: ");
    Serial.print(pm1);
    Serial.print(" ug/m3 | ");

    Serial.print("PM2.5: ");
    Serial.print(pm2_5);
    Serial.print(" ug/m3 | ");

    Serial.print("PM4.0: ");
    Serial.print(pm4);
    Serial.print(" ug/m3 | ");

    Serial.print("PM10.0: ");
    Serial.print(pm10);
    Serial.println(" ug/m3");

    Serial.print("NC0.5: ");
    Serial.print(nc0_5);
    Serial.print(" #/cm3 | ");

    Serial.print("NC1.0: ");
    Serial.print(nc1);
    Serial.print(" #/cm3 | ");

    Serial.print("NC2.5: ");
    Serial.print(nc2_5);
    Serial.print(" #/cm3 | ");

    Serial.print("NC4.0: ");
    Serial.print(nc4);
    Serial.print(" #/cm3 | ");

    Serial.print("NC10.0: ");
    Serial.print(nc10);
    Serial.println(" #/cm3");

    Serial.print(
        "Typical size: "
    );

    Serial.print(
        typicalSize
    );

    Serial.println(" um");

    // Envia os valores ao SensorData.
    copyToData(data);
}


void SPS30::copyToData(
    SensorData& data
) const
{
    data.pm1_0 = pm1;
    data.pm2_5 = pm2_5;
    data.pm4_0 = pm4;
    data.pm10_0 = pm10;

    data.nc0_5 = nc0_5;
    data.nc1_0 = nc1;
    data.nc2_5 = nc2_5;
    data.nc4_0 = nc4;
    data.nc10_0 = nc10;

    data.typicalSize =
        typicalSize;
}