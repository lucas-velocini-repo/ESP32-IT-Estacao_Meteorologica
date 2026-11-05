#include "protocol-handler.h"
#include <ArduinoJson.h>
#include "device/device-identity.h"
#include "config/config.h"

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
    if(strcmp(command, "configure_interval") == 0)
    {
        handleConfigureInterval(doc);
        return;
    }

    if(strcmp(command, "save_identity") == 0)
    {
        handleSaveIdentity(doc);
        return;
    }

    if(strcmp(command, "save_server") == 0)
    {
        handleSaveServer(doc);
        return;
    }

    ble->send("{\"status\":\"error\",\"message\":\"Comando desconhecido\"}" );
}

void ProtocolHandler::handleGetStatus()
{
    JsonDocument response;

    response["type"] = "status";
    response["bluetooth"] = true;
    response["bleFraming"] = true;
    response["stationName"] = settings->getStationName();
    response["server"] = settings->getServer();
    response["intervalSeconds"] = settings->getMeasurementIntervalSeconds();
    response["bleIntervalSupported"] = true;

    response["hardwareId"] =
    DeviceIdentity::getHardwareId();

    response["deviceId"] =
        settings->getDeviceId();

    response["apiTokenConfigured"] =
        settings->getApiToken().length() > 0;

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

void ProtocolHandler::handleSaveIdentity(
    JsonDocument& doc
)
{
    const char* deviceId =
        doc["deviceId"] | "";

    const char* apiToken =
        doc["apiToken"] | "";

    if(strlen(deviceId) == 0)
    {
        ble->send(
            R"({
                "type":"identity_saved",
                "status":"error",
                "message":"deviceId ausente"
            })"
        );

        return;
    }

    if(strlen(apiToken) == 0)
    {
        ble->send(
            R"({
                "type":"identity_saved",
                "status":"error",
                "message":"apiToken ausente"
            })"
        );

        return;
    }

    settings->saveDeviceId(
        String(deviceId)
    );

    settings->saveApiToken(
        String(apiToken)
    );
    settings->saveStationName(String(doc["stationName"] | ""));

    JsonDocument response;

    response["type"] =
        "identity_saved";

    response["status"] =
        "ok";

    response["deviceId"] =
        deviceId;

    std::string json;

    serializeJson(
        response,
        json
    );

    ble->send(json);
}

void ProtocolHandler::handleConfigureInterval(JsonDocument& doc)
{
    JsonDocument response;
    response["type"] = "interval_result";
    response["status"] = "error";
    if (settings->getDeviceId().isEmpty() || settings->getApiToken().isEmpty())
        response["message"] = "Provisione a estação antes de configurar o intervalo.";
    else if (!doc["intervalSeconds"].is<uint32_t>() ||
             doc["intervalSeconds"].as<uint32_t>() < MIN_MEASUREMENT_INTERVAL_SECONDS ||
             doc["intervalSeconds"].as<uint32_t>() > MAX_MEASUREMENT_INTERVAL_SECONDS)
        response["message"] = "Intervalo inválido: use de 30 a 86400 segundos.";
    else if (!settings->saveMeasurementConfiguration(doc["intervalSeconds"].as<uint32_t>(), settings->getConfigurationRevision(), true))
        response["message"] = "Falha ao persistir o intervalo. Tente novamente.";
    else
    {
        response["status"] = "ok";
        response["intervalSeconds"] = settings->getMeasurementIntervalSeconds();
    }
    std::string json;
    serializeJson(response, json);
    ble->send(json);
}

void ProtocolHandler::handleSaveServer(
    JsonDocument& doc
)
{
    const char* server =
        doc["server"] | "";

    if(strlen(server) == 0)
    {
        ble->send(
            R"({
                "type":"server_saved",
                "status":"error",
                "message":"Rota do servidor ausente"
            })"
        );

        return;
    }

    settings->saveServer(
        String(server)
    );

    JsonDocument response;

    response["type"] =
        "server_saved";

    response["status"] =
        "ok";

    std::string json;

    serializeJson(
        response,
        json
    );

    ble->send(json);
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
