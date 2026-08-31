#include "gnss-manager.h"

void GNSSManager::begin(
    HardwareSerial& serialPort,
    int8_t rxPin,
    int8_t txPin,
    uint32_t baudRate
)
{
    serial = &serialPort;

    serial->begin(
        baudRate,
        SERIAL_8N1,
        rxPin,
        txPin
    );

    Serial.println();
    Serial.println("[GNSS] Inicializando NEO-6M...");

    Serial.print("[GNSS] RX do ESP32: GPIO");
    Serial.println(rxPin);

    Serial.print("[GNSS] TX do ESP32: GPIO");
    Serial.println(txPin);

    Serial.print("[GNSS] Baud rate: ");
    Serial.println(baudRate);
}

void GNSSManager::update()
{
    if(serial == nullptr)
    {
        return;
    }

    while(serial->available() > 0)
    {
        char c = static_cast<char>(serial->read());
        gps.encode(c);
    }

    // Atualiza a última posição válida recebida.
    if(
        gps.location.isUpdated()
        && gps.location.isValid()
    )
    {
        latitude = gps.location.lat();
        longitude = gps.location.lng();

        fixValid = true;
        lastFixMillis = millis();

        if(gps.altitude.isValid())
        {
            altitudeMeters =
                static_cast<float>(gps.altitude.meters());
        }

        if(gps.satellites.isValid())
        {
            satellites = gps.satellites.value();
        }

        if(gps.hdop.isValid())
        {
            hdop =
                static_cast<float>(gps.hdop.hdop());
        }

        Serial.print("[GNSS] Fix: ");
        Serial.print(latitude, 6);
        Serial.print(", ");
        Serial.print(longitude, 6);
        Serial.print(" | Satélites: ");
        Serial.println(satellites);
    }

    // Diagnóstico periódico: ajuda a detectar UART sem dados.
    unsigned long now = millis();

    if(
        now - lastDiagnosticMillis >= 5000
    )
    {
        lastDiagnosticMillis = now;

        if(gps.charsProcessed() < 10)
        {
            Serial.println(
                "[GNSS] Nenhum dado NMEA recebido. Verifique alimentação, TX/RX e baud rate."
            );
        }
        else if(!fixValid)
        {
            Serial.println(
                "[GNSS] Dados recebidos, mas ainda sem fix de posição."
            );
        }
    }
}

GNSSData GNSSManager::getData() const
{
    GNSSData data;

    data.valid = fixValid;

    data.latitude = latitude;
    data.longitude = longitude;

    data.altitudeMeters = altitudeMeters;
    data.satellites = satellites;
    data.hdop = hdop;

    if(fixValid)
    {
        data.lastFixAgeMs =
            millis() - lastFixMillis;
    }

    return data;
}

bool GNSSManager::hasFix() const
{
    return fixValid;
}
