#include "gnss-manager.h"

#include "config/config.h"


void GNSSManager::begin(
    HardwareSerial& serialPort,
    int8_t rxPin,
    int8_t txPin,
    uint32_t baudRate
)
{
    serial =
        &serialPort;

    serial->begin(
        baudRate,
        SERIAL_8N1,
        rxPin,
        txPin
    );

    Serial.println();

    Serial.println(
        "[GNSS] Inicializando..."
    );

    Serial.print(
        "[GNSS] RX: GPIO"
    );

    Serial.println(rxPin);

    Serial.print(
        "[GNSS] TX: GPIO"
    );

    Serial.println(txPin);

    Serial.print(
        "[GNSS] Baud: "
    );

    Serial.println(baudRate);
}


void GNSSManager::update()
{
    if(serial == nullptr)
    {
        return;
    }


    while(
        serial->available() > 0
    )
    {
        const char c =
            static_cast<char>(
                serial->read()
            );

        gps.encode(c);
    }


    if(
        gps.location.isUpdated()
        && gps.location.isValid()
    )
    {
        latitude =
            gps.location.lat();

        longitude =
            gps.location.lng();

        fixValid =
            true;

        lastFixMillis =
            millis();


        if(
            gps.altitude.isValid()
        )
        {
            altitudeMeters =
                static_cast<float>(
                    gps.altitude.meters()
                );
        }


        if(
            gps.satellites.isValid()
        )
        {
            satellites =
                gps.satellites.value();
        }


        if(gps.hdop.isValid())
        {
            hdop =
                static_cast<float>(
                    gps.hdop.hdop()
                );
        }


        Serial.print(
            "[GNSS] Fix: "
        );

        Serial.print(
            latitude,
            6
        );

        Serial.print(", ");

        Serial.print(
            longitude,
            6
        );

        Serial.print(
            " | satélites: "
        );

        Serial.println(
            satellites
        );
    }


    /*
     * Diagnóstico apenas uma vez por minuto.
     * Assim uma estação SEM GPS não fica
     * enchendo o Serial de mensagens.
     */
    const unsigned long now =
        millis();


    if(
        now - lastDiagnosticMillis
        >= 60000
    )
    {
        lastDiagnosticMillis =
            now;


        if(
            gps.passedChecksum() == 0
        )
        {
            Serial.println(
                "[GNSS] Nenhuma sentença NMEA válida recebida. GPS pode estar ausente."
            );
        }
        else if(!hasFix())
        {
            Serial.println(
                "[GNSS] GPS detectado, mas ainda sem posição válida."
            );
        }
    }
}


bool GNSSManager::hasFix() const
{
    if(!fixValid)
    {
        return false;
    }


    return (
        millis()
        - lastFixMillis
        <= GNSS_MAX_FIX_AGE_MS
    );
}


GNSSData GNSSManager::getData() const
{
    GNSSData data;

    data.valid =
        hasFix();

    data.latitude =
        latitude;

    data.longitude =
        longitude;

    data.altitudeMeters =
        altitudeMeters;

    data.satellites =
        satellites;

    data.hdop =
        hdop;


    if(fixValid)
    {
        data.lastFixAgeMs =
            millis()
            - lastFixMillis;
    }


    return data;
}