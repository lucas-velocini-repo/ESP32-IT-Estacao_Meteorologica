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

    //wifi.begin(
    //    WIFI_SSID,
    //    WIFI_PASSWORD);
    
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
        else if(strcmp(command, "configure_wifi") == 0)
        {
            const char* ssid = doc["ssid"] | "";

            const char* password = doc["password"] | "";

            const char* server = doc["server"] | "";
            
            if(
                strlen(ssid) == 0 ||
                strlen(password) == 0 ||
                strlen(server) == 0
            )
            {
                ble.send(
                    R"({
                        "type":"configure_result",
                        "status":"error",
                        "message":"Campos ausentes"
                    })"
                );

                return;
            }

            Serial.println();
            Serial.println("Nova configuração:");

            Serial.print("SSID: ");
            Serial.println(ssid);

            Serial.print("Senha: ");
            Serial.println(password);

            Serial.print("Servidor: ");
            Serial.println(server);

            bool connected =
                wifi.configure(
                    ssid,
                    password
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
                response["message"] =
                    "Falha ao conectar WiFi";
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

            return;
        }

        //ble.send("{\"status\":\"ok\"}");
    });
    
    ble.begin();
}

void loop()
{
    //if (!wifi.isConnected())
    //{
    //    Serial.println("WiFi Desconectado");
    //}

    ble.update();

    delay(5000);
}