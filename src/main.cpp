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
#include "device/device-identity.h"
#include "storage/pending-measurement-store.h"
#include "sensors/gnss-manager.h"

WiFiManager wifi;
BLEManager ble;
SettingsManager settings;
ProtocolHandler protocol;
SensorManager sensors;
StationHttpClient stationHttp;
TimeManager timeManager;
PendingMeasurementStore pendingMeasurements;
GNSSManager gnss;

unsigned long lastPendingRetryTime = 0;
constexpr unsigned long PENDING_RETRY_INTERVAL_MS = 2000;
unsigned long lastDataTime = 0;

void processPendingWifiConfiguration();
void processPendingMeasurements();
void sendStationData();

void setup()
{
    Serial.begin(19200);
    settings.begin();
    pendingMeasurements.begin();
    sensors.begin(
        Pins::I2C_SDA,
        Pins::I2C_SCL
    );
    gnss.begin(
        Serial1,
        Pins::GNSS_RX,
        Pins::GNSS_TX,
        GNSS_BAUD_RATE
    );
    stationHttp.begin(settings);
    protocol.begin (ble, wifi, settings);

    String ssid = settings.getSSID();

    String password = settings.getPassword();

    if(ssid.length() > 0)
    {
        wifi.begin(ssid.c_str(), password.c_str());
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

    String hardwareId =
    DeviceIdentity::getHardwareId();

    Serial.print("[Device] Hardware ID: ");
    Serial.println(hardwareId);
}

void loop()
{
    gnss.update();

    ble.update();

    processPendingWifiConfiguration();

    wifi.update();

    timeManager.update();

    processPendingMeasurements();

    unsigned long currentTime = millis();

    if(currentTime - lastDataTime >= DATA_SEND_INTERVAL_MS)
    {
        lastDataTime = currentTime;
        sendStationData();    }

    delay(10);
}


void processPendingWifiConfiguration(){
    if(protocol.hasPendingWifiConfiguration())
    {
        auto config = protocol.takePendingWifiConfiguration();

        Serial.println();
        Serial.println("Aplicando configuração WiFi...");

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

        response["type"] = "configure_result";

        if(connected)
        {
            response["status"] = "ok";

            timeManager.begin();
            timeManager.waitForSynchronization(10000);
        }
        else
        {
            response["status"] = "error";
            response["message"] = "Falha ao conectar WiFi";
        }

        std::string json;

        serializeJson(response, json);

        ble.send(json);

        if(connected)
        {
            JsonDocument status;

            status["type"] = "status";
            status["hardwareId"] = DeviceIdentity::getHardwareId();
            status["deviceId"] = settings.getDeviceId();
            status["apiTokenConfigured"] = settings.getApiToken().length() > 0;
            status["bluetooth"] = true;
            status["wifiConnected"] = true;
            status["ssid"] = wifi.getSSID();
            status["ip"] = wifi.getIP();

            std::string statusJson;

            serializeJson(status, statusJson);

            delay(200);

            ble.send(statusJson);
        }
    }
}

void processPendingMeasurements()
{
    if(!wifi.isConnected())
    {
        return;
    }


    const unsigned long currentTime =
        millis();


    if(
        currentTime
        - lastPendingRetryTime
        < PENDING_RETRY_INTERVAL_MS
    )
    {
        return;
    }


    lastPendingRetryTime =
        currentTime;


    std::string payload;


    if(
        !pendingMeasurements.peek(
            payload
        )
    )
    {
        return;
    }


    Serial.println();
    Serial.print(
        "[Queue] Reenviando medição pendente. Restantes: "
    );

    Serial.println(
        pendingMeasurements.count()
    );


    const bool success =
        stationHttp.send(
            payload
        );


    if(!success)
    {
        Serial.println(
            "[Queue] Reenvio falhou. Medição mantida."
        );

        return;
    }


    if(
        pendingMeasurements.removeFirst()
    )
    {
        Serial.print(
            "[Queue] Medição pendente enviada. Restantes: "
        );

        Serial.println(
            pendingMeasurements.count()
        );
    }
}

void sendStationData()
{
    String deviceId =
        settings.getDeviceId();

    if(deviceId.length() == 0)
    {
        Serial.println(
            "[Station] Device ID não configurado."
        );

        return;
    }

    String apiToken =
        settings.getApiToken();

    if(apiToken.length() == 0)
    {
        Serial.println(
            "[Station] Token de autenticação não configurado."
        );

        return;
    }

    if(
        !timeManager.isSynchronized()
    )
    {
        Serial.println(
            "[Station] Horário ainda não sincronizado. Medição não realizada."
        );

        return;
    }

    SensorData sensorData =
        sensors.read();

    StationData station;

    station.deviceId = deviceId;

    station.deviceName = "Estação Teste";

    station.measuredAt = timeManager.now();

    station.latitude = -23.5;
    station.longitude = -47.2;

    station.sensors = sensorData;

    std::string json =
        JsonBuilder::buildStationPayload(
            station
        );

    Serial.println();
    Serial.println("Payload da estação:");

    Serial.println(
        json.c_str()
    );

    bool success =
        stationHttp.send(
            json
        );


    if(!success)
    {
        Serial.println(
            "[Station] Envio não realizado. Salvando medição localmente..."
        );


        if(
            !pendingMeasurements.enqueue(
                json
            )
        )
        {
            Serial.println(
                "[Station] ERRO: não foi possível salvar a medição pendente."
            );
        }
    }
}