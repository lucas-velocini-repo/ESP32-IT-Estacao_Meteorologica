#pragma once

#include <Arduino.h>
#include <TinyGPS++.h>

struct GNSSData
{
    bool valid = false;

    double latitude = 0.0;
    double longitude = 0.0;

    float altitudeMeters = 0.0f;
    uint32_t satellites = 0;
    float hdop = 0.0f;

    unsigned long lastFixAgeMs = 0;
};

class GNSSManager
{
public:
    void begin(
        HardwareSerial& serialPort,
        int8_t rxPin,
        int8_t txPin,
        uint32_t baudRate = 9600
    );

    // Deve ser chamada com frequência no loop para consumir
    // os dados NMEA recebidos pela UART.
    void update();

    GNSSData getData() const;

    bool hasFix() const;

private:
    HardwareSerial* serial = nullptr;
    TinyGPSPlus gps;

    bool fixValid = false;

    double latitude = 0.0;
    double longitude = 0.0;

    float altitudeMeters = 0.0f;
    uint32_t satellites = 0;
    float hdop = 0.0f;

    unsigned long lastFixMillis = 0;
    unsigned long lastDiagnosticMillis = 0;
};
