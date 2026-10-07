#include "aht20-bmp280.h"

#include <Arduino.h>
#include <Wire.h>
#include <math.h>

void AHT20BMP280::begin()
{
    Serial.println();
    Serial.println("[AHT20/BMP280] Inicializando sensores...");
    
    // AHT20 -----------------------------
    ahtReady = aht.begin(&Wire);

    if(ahtReady)
    {
        Serial.println("[AHT20] Sensor encontrado.");
    }
    else
    {
        Serial.println("[AHT20] ERRO: sensor não encontrado.");
    }

    // BMP280 -----------------------------
    // Primeiro tenta o endereço padrão 0x77.
    bmpReady = bmp.begin(0x77);

    if(bmpReady)
    {
        Serial.println("[BMP280] Sensor encontrado em 0x77.");
    }
    else
    {
        Serial.println("[BMP280] Não encontrado em 0x77.");

        Serial.println("[BMP280] Tentando endereço 0x76...");

        bmpReady = bmp.begin(0x76);

        if(bmpReady)
        {
            Serial.println("[BMP280] Sensor encontrado em 0x76.");
        }
        else
        {
            Serial.println("[BMP280] ERRO: sensor não encontrado.");
        }
    }

    Serial.println("[AHT20/BMP280] Inicialização concluída.");
}

void AHT20BMP280::update(
    SensorData& data
)
{
    // AHT20 -----------------------------
    if(ahtReady)
    {
        sensors_event_t humidityEvent;
        sensors_event_t temperatureEvent;

        bool success =
            aht.getEvent(
                &humidityEvent,
                &temperatureEvent
            );

        if(success)
        {
            if(!isnan(temperatureEvent.temperature))
            {
                temperature = temperatureEvent.temperature;
            }

            if(!isnan(humidityEvent.relative_humidity))
            {
                humidity = humidityEvent.relative_humidity;
            }
        }
        else
        {
            Serial.println("[AHT20] Falha durante leitura.");
        }
    }

    // BMP280 -----------------------------
    if(bmpReady)
    {
        float pressurePa = bmp.readPressure();

        if(!isnan(pressurePa) && pressurePa > 0.0f)
        {
            // BMP280 retorna Pa.
            // O servidor trabalha com hPa.
            pressure = pressurePa / 100.0f;
        }
        else
        {
            Serial.println("[BMP280] Falha durante leitura.");
        }
    }

    data.temperature = temperature;

    data.humidity = humidity;

    data.pressure = pressure;
}