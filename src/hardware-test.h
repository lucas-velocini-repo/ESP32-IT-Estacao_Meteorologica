#pragma once

#include <Arduino.h>
#include "config/config.h"
#include "sensors/sensor-manager.h"
#include "sensors/gnss-manager.h"

namespace HardwareTest
{
    constexpr unsigned long SENSOR_INTERVAL_MS = 2000;
    constexpr unsigned long LED_INTERVAL_MS = 1000;

    constexpr uint8_t RED_PINS[] = {
        Pins::LED1_RED, Pins::LED2_RED, Pins::LED3_RED
    };

    constexpr uint8_t BLUE_PINS[] = {
        Pins::LED1_BLUE, Pins::LED2_BLUE, Pins::LED3_BLUE
    };

    inline void setLeds(uint8_t state)
    {
        // Cátodo comum: HIGH acende.
        // 0 = vermelho, 1 = azul, 2 = ambos.
        for (uint8_t i = 0; i < 3; ++i)
        {
            digitalWrite(RED_PINS[i], state != 1 ? HIGH : LOW);
            digitalWrite(BLUE_PINS[i], state != 0 ? HIGH : LOW);
        }
    }

    inline void begin(SensorManager& sensors, GNSSManager& gps)
    {
        Serial.println("\n=== TESTE DE HARDWARE ===");

        // A inicialização já imprime os dispositivos I2C
        // encontrados e os erros de cada sensor.
        sensors.begin(Pins::I2C_SDA, Pins::I2C_SCL);

        gps.begin(
            Serial1,
            Pins::GNSS_RX,
            Pins::GNSS_TX,
            GNSS_BAUD_RATE
        );

        for (uint8_t i = 0; i < 3; ++i)
        {
            pinMode(RED_PINS[i], OUTPUT);
            pinMode(BLUE_PINS[i], OUTPUT);
        }

        setLeds(0);

        Serial.println("LEDs: vermelho -> azul -> ambos, 1 s cada.");
        Serial.println("Sensores: leitura a cada 2 s.");
        Serial.println("GPS: recepcao continua.");
        Serial.println("Sem WiFi, BLE ou envio ao servidor.\n");
    }

    inline void update(SensorManager& sensors, GNSSManager& gps)
    {
        static unsigned long lastLed = millis();
        static unsigned long lastRead = millis();
        static uint8_t ledState = 0;

        // Processa os dados recebidos do GPS a cada passagem.
        gps.update();

        const unsigned long now = millis();

        if (now - lastLed >= LED_INTERVAL_MS)
        {
            lastLed = now;
            ledState = (ledState + 1) % 3;
            setLeds(ledState);
        }

        if (now - lastRead < SENSOR_INTERVAL_MS)
            return;

        lastRead = now;

        Serial.println("\n========== SENSORES ==========");

        const SensorData data = sensors.read();

        Serial.printf("Temperatura: %.2f C\n", data.temperature);
        Serial.printf("Umidade:     %.2f %%\n", data.humidity);
        Serial.printf("Pressao:     %.2f hPa\n", data.pressure);
        Serial.printf("Luz:         %.2f lx\n", data.light);

        // O driver SPS30 já imprime PM, concentração
        // de partículas e tamanho típico ao ler.

        const GNSSData position = gps.getData();

        Serial.println("============ GPS =============");

        if (position.valid)
        {
            Serial.printf(
                "Latitude: %.6f | Longitude: %.6f\n",
                position.latitude,
                position.longitude
            );

            Serial.printf(
                "Altitude: %.1f m | Satelites: %lu | HDOP: %.2f\n",
                position.altitudeMeters,
                static_cast<unsigned long>(position.satellites),
                position.hdop
            );

            Serial.printf(
                "Idade da posicao: %lu ms\n",
                position.lastFixAgeMs
            );
        }
        else
        {
            Serial.println("Aguardando posicao valida...");
        }

        Serial.println("==============================\n");
    }
}