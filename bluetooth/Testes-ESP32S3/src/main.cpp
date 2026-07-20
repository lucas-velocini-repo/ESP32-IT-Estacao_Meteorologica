#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include "network/wifi-manager.h"
#include "config/config.h"
#include "bluetooth/ble-manager.h"

WiFiManager wifi;
BLEManager ble;

void setup()
{
    Serial.begin(19200);

    wifi.begin(
        WIFI_SSID,
        WIFI_PASSWORD);
    
    ble.setMessageCallback(
    [&](const std::string& msg)
    {
        Serial.println();
        Serial.println("JSON recebido:");

        Serial.println(msg.c_str());

        JsonDocument doc;

        DeserializationError error =
            deserializeJson(doc, msg);

        if(error)
        {
            Serial.println("JSON inválido.");

            ble.send("{\"status\":\"error\", \"message\":\"JSON inválido\"}");

            return;
        }

        if(!doc["command"].is<const char*>())
        {
            ble.send(
                "{\"status\":\"error\",\"message\":\"Campo command ausente\"}"
            );

            return;
        }

        const char* command = doc["command"];
        
        Serial.print("Command: ");
        Serial.println(command);

        if(strcmp(command, "get_status") == 0)
        {
            JsonDocument response;
            response["type"] = "status";
            response["bluetooth"] = true;

            bool wifiConnected = wifi.isConnected();

            response["wifiConnected"] =
                wifiConnected;

            if(wifiConnected)
            {
                response["ssid"] = wifi.getSSID();
                response["ip"] = wifi.getIP();
            }
            else
            {
                response["ssid"] = "";
                response["ip"] = "";
            }

            std::string json;

            serializeJson(response, json);

            ble.send(json);

            return;
        }

        ble.send("{\"status\":\"ok\"}");
    });
    
    ble.begin();
}

void loop()
{
    if (!wifi.isConnected())
    {
        Serial.println("WiFi Desconectado");
    }

    ble.update();

    delay(5000);
}