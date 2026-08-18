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

WiFiManager wifi;
BLEManager ble;
SettingsManager settings;
ProtocolHandler protocol;
SensorManager sensors;
StationHttpClient stationHttp;
TimeManager timeManager;
unsigned long lastDataTime = 0;
void processPendingWifiConfiguration();
void sendStationData();

void setup()
{
    Serial.begin(19200);
    settings.begin();
    sensors.begin();
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
}

void loop()
{
    ble.update();

    processPendingWifiConfiguration();

    timeManager.update();

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

void sendStationData()
{
    SensorData sensorData = sensors.read();

    StationData station;

    station.deviceId = 6;
    station.deviceName = "estacao maratonista";

    station.measuredAt = timeManager.now();

    if(station.measuredAt == 0)
    {
        Serial.println("[Time] Horário ainda não sincronizado.");
        Serial.println("[Time] O servidor usará received_at como fallback.");
    }

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

    bool success = stationHttp.send(json);

    if(!success)
    {
        Serial.println("[Estação] Envio não realizado.");
    }
}