#include "power-manager.h"

#include <Arduino.h>
#include <esp_sleep.h>
#include <esp_timer.h>
#include <driver/rtc_io.h>
#include <esp_arduino_version.h>
#include "config/config.h"

namespace
{
    uint32_t normalFrequencyMhz = 240;
    constexpr bool GNSS_ENABLE_ACTIVE_HIGH = true;
}

void PowerManager::begin()
{
    pinMode(Pins::POWER_SOURCE, INPUT);
    normalFrequencyMhz = getCpuFrequencyMhz();
    if (Pins::GNSS_POWER_ENABLE >= 0)
    {
        digitalWrite(Pins::GNSS_POWER_ENABLE, GNSS_ENABLE_ACTIVE_HIGH ? LOW : HIGH);
        pinMode(Pins::GNSS_POWER_ENABLE, OUTPUT);
    }
    gnssPower(false);
    internalLedsOff();
}

bool PowerManager::externalPower()
{
    return digitalRead(Pins::POWER_SOURCE) == HIGH;
}

void PowerManager::internalLedsOff()
{
    // Um quadro preto apaga o RGB endereçável; somente LOW não limpa sua memória.
#if ESP_ARDUINO_VERSION_MAJOR >= 3
    rgbLedWrite(Pins::INTERNAL_RGB_LED, 0, 0, 0);
#else
    neopixelWrite(Pins::INTERNAL_RGB_LED, 0, 0, 0);
#endif
    pinMode(Pins::INTERNAL_RGB_LED, OUTPUT);
    digitalWrite(Pins::INTERNAL_RGB_LED, LOW);
}

void PowerManager::gnssPower(bool on)
{
    if (!on)
    {
        // Não alimenta o GPS desligado pelos diodos da UART.
        pinMode(Pins::GNSS_RX, INPUT);
        pinMode(Pins::GNSS_TX, INPUT);
    }
    if (Pins::GNSS_POWER_ENABLE >= 0)
        digitalWrite(Pins::GNSS_POWER_ENABLE, on == GNSS_ENABLE_ACTIVE_HIGH ? HIGH : LOW);
}

void PowerManager::batteryCpu()
{
    setCpuFrequencyMhz(80);
}

void PowerManager::normalCpu()
{
    setCpuFrequencyMhz(normalFrequencyMhz);
}

uint64_t PowerManager::nowMs()
{
    return static_cast<uint64_t>(esp_timer_get_time()) / 1000;
}

void PowerManager::idleBattery(uint32_t milliseconds, bool redLedOn)
{
    if (milliseconds == 0 || externalPower()) return;
    if (redLedOn || milliseconds < 5)
    {
        // LEDC pára no light sleep. Durante o pulso mantém o PWM com CPU a 80 MHz.
        delay(milliseconds < 10 ? milliseconds : 10);
        return;
    }
    Serial.flush();
    esp_sleep_enable_timer_wakeup(static_cast<uint64_t>(milliseconds) * 1000);
    // GPIO7 é RTC IO no ESP32-S3: a fonte externa acorda o processador.
    esp_sleep_enable_ext0_wakeup(static_cast<gpio_num_t>(Pins::POWER_SOURCE), 1);
    if (!externalPower()) esp_light_sleep_start();
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_EXT0);
    // EXT0 deixa o pino no mux RTC; devolve ao GPIO antes do próximo digitalRead.
    rtc_gpio_deinit(static_cast<gpio_num_t>(Pins::POWER_SOURCE));
    pinMode(Pins::POWER_SOURCE, INPUT);
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TIMER);
}
