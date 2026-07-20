#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include "network/wifi-manager.h"
#include "config/config.h"
#include "bluetooth/ble-manager.h"
#include "config/settings-manager.h"
#include "protocol/protocol-handler.h"

WiFiManager wifi;
BLEManager ble;
SettingsManager settings;
ProtocolHandler protocol;
void processPendingWifiConfiguration();

void setup()
{
    Serial.begin(19200);
    settings.begin();
    protocol.begin (ble, wifi,settings);

    String ssid = settings.getSSID();

    String password = settings.getPassword();

    if(ssid.length() > 0)
    {
        wifi.begin(ssid.c_str(), password.c_str());
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