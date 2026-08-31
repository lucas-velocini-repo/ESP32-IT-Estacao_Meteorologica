#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>

#include "network/wifi-manager.h"
#include "config/config.h"
#include "bluetooth/ble-manager.h"
#include "config/settings-manager.h"
#include "protocol/protocol-handler.h"
#include "sensors/sensor-manager.h"
#include "api/json-builder.h"
#include "station/station-data.h"
#include "network/station-http-client.h"
#include "time/time-manager.h"
#include "sensors/gnss-manager.h"

// ======================================================
// PINAGEM DO PROTÓTIPO
// Todas as GPIOs usadas pelo projeto ficam definidas aqui.
// ======================================================

namespace Pins
{
    // I2C - AHT20, BMP280, BH1750 e SPS30
    constexpr uint8_t I2C_SDA = 8;
    constexpr uint8_t I2C_SCL = 9;

    // UART1 - GNSS NEO-6M
    // Ligação física:
    // NEO-6M TX -> ESP32 GPIO18 (RX)
    // NEO-6M RX -> ESP32 GPIO17 (TX)
    constexpr uint8_t GNSS_TX = 17;
    constexpr uint8_t GNSS_RX = 18;

    // HIGH = fonte externa
    // LOW  = bateria
    constexpr uint8_t POWER_SOURCE = 7;

    // LED bicolor 1
    constexpr uint8_t LED1_RED = 4;
    constexpr uint8_t LED1_BLUE = 5;

    // LED bicolor 2
    constexpr uint8_t LED2_RED = 11;
    constexpr uint8_t LED2_BLUE = 12;

    // LED bicolor 3
    constexpr uint8_t LED3_RED = 13;
    constexpr uint8_t LED3_BLUE = 14;
}

constexpr uint32_t GNSS_BAUD_RATE = 9600;

// ======================================================
// OBJETOS DO SISTEMA
// ======================================================

WiFiManager wifi;
BLEManager ble;
SettingsManager settings;
ProtocolHandler protocol;
SensorManager sensors;
StationHttpClient stationHttp;
TimeManager timeManager;
GNSSManager gnss;

// ======================================================
// ESTADO GLOBAL
// ======================================================

unsigned long lastDataTime = 0;

// true  -> estação alimentada pela fonte externa
// false -> estação alimentada pela bateria
bool powerOnMains = false;

// Usado apenas para imprimir mudança de fonte no Serial.
bool powerSourceInitialized = false;

// ======================================================
// FUNÇÕES
// ======================================================

void processPendingWifiConfiguration();
void sendStationData();
void updatePowerSource();
void initializeStatusLeds();

void setBicolorLed(
    uint8_t redPin,
    uint8_t bluePin,
    bool redOn,
    bool blueOn
);

// ======================================================
// SETUP
// ======================================================

void setup()
{
    Serial.begin(19200);

    // --------------------------------------------------
    // Entrada de detecção da alimentação
    // O divisor resistivo externo já define HIGH/LOW,
    // portanto não é usado INPUT_PULLUP.
    // --------------------------------------------------

    pinMode(
        Pins::POWER_SOURCE,
        INPUT
    );

    updatePowerSource();

    // --------------------------------------------------
    // LEDs bicolores
    // --------------------------------------------------

    initializeStatusLeds();

    // --------------------------------------------------
    // Sensores I2C
    // --------------------------------------------------

    sensors.begin(
        Pins::I2C_SDA,
        Pins::I2C_SCL
    );

    // --------------------------------------------------
    // GNSS NEO-6M na UART1
    // --------------------------------------------------

    gnss.begin(
        Serial1,
        Pins::GNSS_RX,
        Pins::GNSS_TX,
        GNSS_BAUD_RATE
    );

    // --------------------------------------------------
    // Demais módulos do sistema
    // --------------------------------------------------

    settings.begin();

    stationHttp.begin(settings);

    protocol.begin(
        ble,
        wifi,
        settings
    );

    String ssid = settings.getSSID();
    String password = settings.getPassword();

    if(ssid.length() > 0)
    {
        wifi.begin(
            ssid.c_str(),
            password.c_str()
        );
    }

    timeManager.begin();

    if(wifi.isConnected())
    {
        timeManager.waitForSynchronization(
            10000
        );
    }

    ble.setMessageCallback(
        [&](const std::string& msg)
        {
            protocol.handle(msg);
        }
    );

    ble.begin();
}

// ======================================================
// LOOP
// ======================================================

void loop()
{
    // Mantém o estado da alimentação atualizado.
    updatePowerSource();

    // O GNSS envia NMEA continuamente. É importante
    // consumir a UART frequentemente para não perder dados.
    gnss.update();

    ble.update();

    processPendingWifiConfiguration();

    timeManager.update();

    unsigned long currentTime = millis();

    if(
        currentTime - lastDataTime
        >= DATA_SEND_INTERVAL_MS
    )
    {
        lastDataTime = currentTime;
        sendStationData();
    }

    delay(10);
}

// ======================================================
// ALIMENTAÇÃO
// ======================================================

void updatePowerSource()
{
    bool newPowerOnMains =
        digitalRead(Pins::POWER_SOURCE) == HIGH;

    if(
        !powerSourceInitialized
        || newPowerOnMains != powerOnMains
    )
    {
        powerOnMains = newPowerOnMains;
        powerSourceInitialized = true;

        Serial.print("[Power] Alimentação: ");

        if(powerOnMains)
        {
            Serial.println("FONTE");
        }
        else
        {
            Serial.println("BATERIA");
        }
    }
    else
    {
        powerOnMains = newPowerOnMains;
    }
}

// ======================================================
// LEDs
// ======================================================

void initializeStatusLeds()
{
    pinMode(Pins::LED1_RED, OUTPUT);
    pinMode(Pins::LED1_BLUE, OUTPUT);

    pinMode(Pins::LED2_RED, OUTPUT);
    pinMode(Pins::LED2_BLUE, OUTPUT);

    pinMode(Pins::LED3_RED, OUTPUT);
    pinMode(Pins::LED3_BLUE, OUTPUT);

    // Todos começam apagados.
    setBicolorLed(
        Pins::LED1_RED,
        Pins::LED1_BLUE,
        false,
        false
    );

    setBicolorLed(
        Pins::LED2_RED,
        Pins::LED2_BLUE,
        false,
        false
    );

    setBicolorLed(
        Pins::LED3_RED,
        Pins::LED3_BLUE,
        false,
        false
    );
}

void setBicolorLed(
    uint8_t redPin,
    uint8_t bluePin,
    bool redOn,
    bool blueOn
)
{
    // Os LEDs usados são de cátodo comum:
    // HIGH no GPIO acende a respectiva cor.
    digitalWrite(
        redPin,
        redOn ? HIGH : LOW
    );

    digitalWrite(
        bluePin,
        blueOn ? HIGH : LOW
    );
}

// ======================================================
// CONFIGURAÇÃO WIFI
// ======================================================

void processPendingWifiConfiguration()
{
    if(protocol.hasPendingWifiConfiguration())
    {
        auto config =
            protocol.takePendingWifiConfiguration();

        Serial.println();
        Serial.println(
            "Aplicando configuração WiFi..."
        );

        settings.saveWifi(
            config.ssid.c_str(),
            config.password.c_str(),
            config.server.c_str()
        );

        bool connected =
            wifi.configure(
                config.ssid.c_str(),
                config.password.c_str()
            );

        JsonDocument response;

        response["type"] =
            "configure_result";

        if(connected)
        {
            response["status"] = "ok";

            timeManager.begin();

            timeManager.waitForSynchronization(
                10000
            );
        }
        else
        {
            response["status"] = "error";
            response["message"] =
                "Falha ao conectar WiFi";
        }

        std::string json;

        serializeJson(
            response,
            json
        );

        ble.send(json);

        if(connected)
        {
            JsonDocument status;

            status["type"] = "status";
            status["bluetooth"] = true;
            status["wifiConnected"] = true;
            status["ssid"] = wifi.getSSID();
            status["ip"] = wifi.getIP();

            std::string statusJson;

            serializeJson(
                status,
                statusJson
            );

            delay(200);

            ble.send(statusJson);
        }
    }
}

// ======================================================
// LEITURA E ENVIO DA ESTAÇÃO
// ======================================================

void sendStationData()
{
    SensorData sensorData =
        sensors.read();

    StationData station;

    station.deviceId = 7;
    station.deviceName = "estacao legal";

    station.measuredAt =
        timeManager.now();

    if(station.measuredAt == 0)
    {
        Serial.println(
            "[Time] Horário ainda não sincronizado."
        );

        Serial.println(
            "[Time] O servidor usará received_at como fallback."
        );
    }

    // --------------------------------------------------
    // Posição fornecida pelo NEO-6M
    // --------------------------------------------------

    GNSSData gnssData =
        gnss.getData();

    if(gnssData.valid)
    {
        station.latitude =
            gnssData.latitude;

        station.longitude =
            gnssData.longitude;
    }
    else
    {
        // Enquanto não houver fix, a estrutura permanece
        // com latitude/longitude = 0.0 em vez de enviar
        // coordenadas fictícias.
        station.latitude = 0.0;
        station.longitude = 0.0;

        Serial.println(
            "[GNSS] Sem posição válida no momento do envio."
        );
    }

    station.sensors =
        sensorData;

    // A variável powerOnMains mantém o estado solicitado:
    // true = fonte externa / false = bateria.
    Serial.print("[Power] Estado armazenado: ");
    Serial.println(
        powerOnMains
        ? "FONTE"
        : "BATERIA"
    );

    std::string json =
        JsonBuilder::buildStationPayload(
            station
        );

    Serial.println();
    Serial.println(
        "Payload da estação:"
    );

    Serial.println(
        json.c_str()
    );

    bool success =
        stationHttp.send(json);

    if(!success)
    {
        Serial.println(
            "[Estação] Envio não realizado."
        );
    }
}
