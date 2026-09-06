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
    constexpr uint8_t I2C_SDA = 5;
    constexpr uint8_t I2C_SCL = 4;


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
    // Apenas reservado por enquanto.
    // ==================================================

    constexpr uint8_t POWER_SOURCE = 7;


    // ==================================================
    // LEDs BICOLORES
    //
    // ATENÇÃO:
    // Estes pinos são da montagem do Gustavo.
    // Na montagem atual I2C usa GPIO 4 e 5.
    //
    // Portanto NÃO inicializaremos os LEDs ainda.
    // ==================================================

    constexpr uint8_t LED1_RED = 4;
    constexpr uint8_t LED1_BLUE = 5;

    constexpr uint8_t LED2_RED = 11;
    constexpr uint8_t LED2_BLUE = 12;

    constexpr uint8_t LED3_RED = 13;
    constexpr uint8_t LED3_BLUE = 14;
}


// ======================================================
// GNSS
// ======================================================

constexpr uint32_t
    GNSS_BAUD_RATE = 9600;

constexpr unsigned long
    GNSS_MAX_FIX_AGE_MS =
        300000;