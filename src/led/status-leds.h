#pragma once

#include <Arduino.h>
#include "config/config.h"

namespace StatusLeds
{
    inline void begin()
    {
        // O circuito externo deve fornecer HIGH ou LOW.
        pinMode(Pins::POWER_SOURCE, INPUT);

        const uint8_t pins[] = {
            Pins::LED1_RED, Pins::LED1_BLUE,
            Pins::LED2_RED, Pins::LED2_BLUE,
            Pins::LED3_RED, Pins::LED3_BLUE
        };

        for (uint8_t pin : pins)
        {
            digitalWrite(pin, LOW);
            pinMode(pin, OUTPUT);
        }
    }

    inline void update(bool wifiConnected, bool bluetoothConnected)
    {
        // LEDs de cátodo comum: HIGH acende.

        // LED 1: Bluetooth conectado.
        digitalWrite(Pins::LED1_RED, LOW);
        digitalWrite(
            Pins::LED1_BLUE,
            bluetoothConnected ? HIGH : LOW
        );

        // LED 2: Wi-Fi conectado.
        digitalWrite(Pins::LED2_RED, LOW);
        digitalWrite(
            Pins::LED2_BLUE,
            wifiConnected ? HIGH : LOW
        );

        // LED 3: fonte de alimentação.
        const bool powerHigh =
            digitalRead(Pins::POWER_SOURCE) == HIGH;

        const unsigned long now = millis();

        static bool wasPowerHigh = true;
        static unsigned long cycleStart = 0;

        if (powerHigh)
        {
            digitalWrite(Pins::LED3_RED, LOW);
            digitalWrite(Pins::LED3_BLUE, HIGH);
        }
        else
        {
            // Ao passar para LOW, começa com vermelho aceso.
            if (wasPowerHigh)
            {
                cycleStart = now;
            }

            unsigned long elapsed = now - cycleStart;

            // Ciclo de 6 s: 1 s aceso e 5 s apagado.
            if (elapsed >= 6000UL)
            {
                cycleStart += (elapsed / 6000UL) * 6000UL;
                elapsed = now - cycleStart;
            }

            digitalWrite(Pins::LED3_BLUE, LOW);
            digitalWrite(
                Pins::LED3_RED,
                elapsed < 1000UL ? HIGH : LOW
            );
        }

        wasPowerHigh = powerHigh;
    }
}