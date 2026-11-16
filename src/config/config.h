#pragma once

#include <Arduino.h>


// ======================================================
// IDENTIFICAÇÃO
// ======================================================

#define DEVICE_NAME "ESP-PILOTO"


// ======================================================
// INTERVALOS
// ======================================================

constexpr unsigned long
    DATA_SEND_INTERVAL_MS = 300000;

// ======================================================
// I2C
// AHT20, BMP280, BH1750 e SPS30
// ======================================================

namespace Pins
{
    constexpr uint8_t I2C_SDA = 8;
    constexpr uint8_t I2C_SCL = 9;


    // ==================================================
    // GNSS / GPS
    //
    // GPS TX -> ESP GNSS_RX
    // GPS RX -> ESP GNSS_TX
    // ==================================================

    constexpr uint8_t GNSS_RX = 18;
    constexpr uint8_t GNSS_TX = 17;


    // ==================================================
    // DETECÇÃO DE ALIMENTAÇÃO
    // HIGH = fonte externa; LOW = bateria. Entrada de 3,3 V, nunca flutuante.
    // ==================================================

    constexpr uint8_t POWER_SOURCE = 7;


    // ==================================================
    // LEDs BICOLORES
    //
    // Pinagem preservada dos sources. Confira a fiação antes do upload.
    // Cátodo comum: cada anodo usa seu próprio resistor.
    // ==================================================

    constexpr uint8_t LED1_RED = 4;
    constexpr uint8_t LED1_BLUE = 5;

    constexpr uint8_t LED2_RED = 11;
    constexpr uint8_t LED2_BLUE = 12;

    constexpr uint8_t LED3_RED = 13;
    constexpr uint8_t LED3_BLUE = 14;

    constexpr int8_t INTERNAL_RGB_LED = 48;
    // -1 = não existe controle elétrico instalado. Use EN/load switch,
    // nunca ligue o VCC do GPS diretamente a um GPIO.
    constexpr int8_t GNSS_POWER_ENABLE = -1;
}


// ======================================================
// GNSS
// ======================================================

constexpr uint32_t
    GNSS_BAUD_RATE = 9600;

constexpr unsigned long
    GNSS_MAX_FIX_AGE_MS =
        300000;
constexpr uint32_t MIN_MEASUREMENT_INTERVAL_SECONDS = 30;
constexpr uint32_t MAX_MEASUREMENT_INTERVAL_SECONDS = 86400;
constexpr unsigned long CONFIG_POLL_INTERVAL_MS = 60000;
constexpr const char* FIRMWARE_VERSION = "1.4.0-dev";

namespace BatteryConfig
{
    constexpr uint32_t SPS30_WARMUP_MS = 30000;
    constexpr uint32_t SPS30_READ_TIMEOUT_MS = 5000;
    constexpr uint32_t MIN_SLEEP_MS = 30000;
    constexpr uint32_t LIGHT_CONVERSION_MS = 180;
    constexpr uint32_t LED_PERIOD_MS = 5000; // 1 s ligado + 4 s apagado
    constexpr uint32_t LED_ON_MS = 1000;
    constexpr uint32_t POWER_DEBOUNCE_MS = 50;
}
