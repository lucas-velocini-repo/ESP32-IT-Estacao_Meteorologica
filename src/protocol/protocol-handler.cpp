#include "protocol-handler.h"
#include <ArduinoJson.h>

void ProtocolHandler::begin(
    BLEManager& ble,
    WiFiManager& wifi,
    SettingsManager& settings
)
{
    this->ble = &ble;
    this->wifi = &wifi;
    this->settings = &settings;
}

void ProtocolHandler::handle(
    const std::string& message
)
{
    JsonDocument doc;

    DeserializationError error =
        deserializeJson(doc, message);

    if(error)
    {
        ble->send( "{\"status\":\"error\",\"message\":\"JSON inválido\"}");
        return;
    }

    if(!doc["command"].is<const char*>())
    {
        ble->send( "{\"status\":\"error\",\"message\":\"Campo command ausente\"}");
        return;
    }

    const char* command = doc["command"];

    Serial.print("Command: ");
    Serial.println(command);

    if(strcmp(command, "get_status") == 0)
    {
        handleGetStatus();
        return;
    }

    if(strcmp(command, "configure_wifi") == 0)
    {
        handleConfigureWifi(doc);
        return;
    }

    ble->send("{\"status\":\"error\",\"message\":\"Comando desconhecido\"}" );
}

void ProtocolHandler::handleGetStatus()
{
    JsonDocument response;

    response["type"] = "status";
    response["bluetooth"] = true;

    bool wifiConnected = wifi->isConnected();

    response["wifiConnected"] = wifiConnected;

    if(wifiConnected)
    {
        response["ssid"] = wifi->getSSID();
        response["ip"] = wifi->getIP();
    }
    else
    {
        response["ssid"] = "";
        response["ip"] = "";
    }

    std::string json;

    serializeJson(
        response,
        json
    );

    ble->send(json);
}

void ProtocolHandler::handleConfigureWifi(JsonDocument& doc)
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
        ble->send(
            R"({
                "type":"configure_result",
                "status":"error",
                "message":"Campos ausentes"
            })"
        );

        return;
    }

    pendingWifiConfig.ssid = ssid;
    pendingWifiConfig.password = password;
    pendingWifiConfig.server = server;

    pendingWifiConfig.pending = true;
}

bool ProtocolHandler::hasPendingWifiConfiguration() const
{
    return pendingWifiConfig.pending;
}

ProtocolHandler::PendingWifiConfig
ProtocolHandler::takePendingWifiConfiguration()
{
    PendingWifiConfig config = pendingWifiConfig;
    pendingWifiConfig.pending = false;
    return config;
}
