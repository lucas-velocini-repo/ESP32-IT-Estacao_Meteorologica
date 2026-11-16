#pragma once

#include <Arduino.h>
#include <initializer_list>
#include "config/config.h"

namespace StatusLeds
{
    // Edite estes valores (0 a 255). PWM não substitui o resistor em cada anodo.
    inline void write(uint8_t pin, bool on, bool red)
    {
        constexpr uint8_t RED_BRIGHTNESS = 48;
        constexpr uint8_t BLUE_BRIGHTNESS = 20;
        const uint16_t duty = on ? (red ? RED_BRIGHTNESS : BLUE_BRIGHTNESS) : 0;
        // GPIO0..48 no S3. Evita reconfigurar o LEDC a cada passagem no loop.
        static uint16_t previous[49] = {};
        if (pin >= 49 || previous[pin] == duty + 1) return;
        analogWrite(pin, duty);
        previous[pin] = duty + 1;
    }

    inline void allOff()
    {
        for (uint8_t pin : {Pins::LED1_RED, Pins::LED2_RED, Pins::LED3_RED})
            write(pin, false, true);
        for (uint8_t pin : {Pins::LED1_BLUE, Pins::LED2_BLUE, Pins::LED3_BLUE})
            write(pin, false, false);
    }

    inline void begin()
    {
        for (uint8_t pin : {Pins::LED1_RED, Pins::LED1_BLUE, Pins::LED2_RED,
                           Pins::LED2_BLUE, Pins::LED3_RED, Pins::LED3_BLUE})
        {
            digitalWrite(pin, LOW);
            pinMode(pin, OUTPUT);
        }
        allOff();
    }

    inline void normal(bool wifiConnected, bool bluetoothConnected)
    {
        write(Pins::LED1_RED, false, true);
        write(Pins::LED1_BLUE, bluetoothConnected, false);
        write(Pins::LED2_RED, false, true);
        write(Pins::LED2_BLUE, wifiConnected, false);
        write(Pins::LED3_RED, false, true);
        write(Pins::LED3_BLUE, true, false);
    }

    inline bool battery(uint64_t elapsedMs)
    {
        write(Pins::LED1_RED, false, true);
        write(Pins::LED1_BLUE, false, false);
        write(Pins::LED2_RED, false, true);
        write(Pins::LED2_BLUE, false, false);
        write(Pins::LED3_BLUE, false, false);
        const bool on = elapsedMs % BatteryConfig::LED_PERIOD_MS < BatteryConfig::LED_ON_MS;
        write(Pins::LED3_RED, on, true);
        return on;
    }
}
