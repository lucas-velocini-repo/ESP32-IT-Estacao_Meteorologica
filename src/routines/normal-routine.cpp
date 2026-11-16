#include "normal-routine.h"

#include <ArduinoJson.h>
#include "api/json-builder.h"
#include "config/config.h"
#include "device/device-identity.h"
#include "led/status-leds.h"
#include "power/power-manager.h"

void NormalRoutine::begin()
{
    if (!PowerManager::externalPower()) return;
    PowerManager::normalCpu();
    station.sensors.setBatteryMode(false);
    PowerManager::gnssPower(true);
    station.gnss.begin(Serial1, Pins::GNSS_RX, Pins::GNSS_TX, GNSS_BAUD_RATE);
    stationHttp.begin(station.settings);
    protocol.begin(ble, wifi, station.settings);
    ble.setMessageCallback([this](const std::string& message) {
        if (PowerManager::externalPower()) protocol.handle(message);
    });
    ble.begin();
    const String ssid = station.settings.getSSID();
    const String password = station.settings.getPassword();
    if (!ssid.isEmpty() && PowerManager::externalPower())
        wifi.begin(ssid.c_str(), password.c_str());
    if (PowerManager::externalPower()) station.clock.begin();
    lastDataTime = millis();
    lastPendingRetryTime = 0;
    configurationAttempted = false;
    Serial.print("[Device] Hardware ID: ");
    Serial.println(DeviceIdentity::getHardwareId());
    updateStatusLeds();
}

void NormalRoutine::end()
{
    // Para reconexão, publicidade BLE, SNTP e UART antes de entrar na bateria.
    ble.end();
    wifi.end();
    station.clock.end();
    station.gnss.end();
    PowerManager::gnssPower(false);
    StatusLeds::allOff();
    if (protocol.hasPendingWifiConfiguration()) protocol.takePendingWifiConfiguration();
}

void NormalRoutine::update()
{
    if (!PowerManager::externalPower()) return;
    station.gnss.update();
    ble.update();
    processPendingWifiConfiguration();
    if (!PowerManager::externalPower()) return;
    wifi.update();
    station.clock.update();
    updateStatusLeds();
    station.location.update(station.gnss, station.clock, station.settings, stationHttp);
    if (!PowerManager::externalPower()) return;
    synchronizeMeasurementConfiguration();
    if (!PowerManager::externalPower()) return;
    processPendingMeasurements();
    if (!PowerManager::externalPower()) return;
    const unsigned long now = millis();
    if (now - lastDataTime >= station.settings.getMeasurementIntervalSeconds() * 1000UL)
    {
        lastDataTime = now;
        sendStationData();
    }
    updateStatusLeds();
    delay(10);
}

void NormalRoutine::updateStatusLeds()
{
    if (PowerManager::externalPower())
        StatusLeds::normal(wifi.isConnected(), ble.isConnected());
    else StatusLeds::allOff();
}

void NormalRoutine::processPendingWifiConfiguration()
{
    if (PowerManager::externalPower() && protocol.hasPendingWifiConfiguration())
    {
        auto config = protocol.takePendingWifiConfiguration();

        Serial.println();
        Serial.println("Aplicando configuração WiFi...");

        station.settings.saveWifi(
            config.ssid.c_str(),
            config.password.c_str(),
            config.server.c_str()
        );

        bool connected = wifi.configure(
            config.ssid.c_str(),
            config.password.c_str()
        );

        updateStatusLeds();
        if (!PowerManager::externalPower()) return;

        JsonDocument response;

        response["type"] = "configure_result";

        if (connected)
        {
            response["status"] = "ok";

            station.clock.begin();
            // NTP é acompanhado no loop para não atrasar a troca de alimentação.

            updateStatusLeds();
        }
        else
        {
            response["status"] = "error";
            response["message"] = "Falha ao conectar WiFi";
        }

        std::string json;
        serializeJson(response, json);

        ble.send(json);

        if (connected)
        {
            delay(200);
            protocol.sendStatus();
        }

        updateStatusLeds();
    }
}

void NormalRoutine::processPendingMeasurements()
{
    if (!wifi.isConnected())
    {
        return;
    }

    const unsigned long currentTime = millis();

    if (
        currentTime - lastPendingRetryTime
        < 2000UL
    )
    {
        return;
    }

    lastPendingRetryTime = currentTime;

    std::string payload;

    if (!station.pendingMeasurements.peek(payload))
    {
        return;
    }

    Serial.println();
    Serial.print(
        "[Queue] Reenviando medição pendente. Restantes: "
    );
    Serial.println(station.pendingMeasurements.count());

    const bool success = stationHttp.send(payload);

    updateStatusLeds();

    if (!success)
    {
        Serial.println(
            "[Queue] Reenvio falhou. Medição mantida."
        );

        return;
    }

    if (station.pendingMeasurements.removeFirst())
    {
        Serial.print(
            "[Queue] Medição pendente enviada. Restantes: "
        );
        Serial.println(station.pendingMeasurements.count());
    }
}

void NormalRoutine::sendStationData()
{
    String deviceId = station.settings.getDeviceId();

    if (deviceId.length() == 0)
    {
        Serial.println(
            "[Station] Device ID não configurado."
        );

        return;
    }

    String apiToken = station.settings.getApiToken();

    if (apiToken.length() == 0)
    {
        Serial.println(
            "[Station] Token de autenticação não configurado."
        );

        return;
    }

    if (!station.clock.isSynchronized())
    {
        Serial.println(
            "[Station] Horário ainda não sincronizado. "
            "Medição não realizada."
        );

        return;
    }

    const StationData measurement = station.measurement(station.sensors.read());

    std::string json = JsonBuilder::buildStationPayload(measurement);

    Serial.println();
    Serial.println("Payload da estação:");
    Serial.println(json.c_str());

    bool success = stationHttp.send(json);

    updateStatusLeds();

    if (!success)
    {
        Serial.println(
            "[Station] Envio não realizado. "
            "Salvando medição localmente..."
        );

        if (!station.pendingMeasurements.enqueue(json))
        {
            Serial.println(
                "[Station] ERRO: não foi possível "
                "salvar a medição pendente."
            );
        }
    }
}

void NormalRoutine::synchronizeMeasurementConfiguration()
{
    if (!wifi.isConnected() || station.settings.getDeviceId().isEmpty() || station.settings.getApiToken().isEmpty()) return;
    const unsigned long now = millis();
    if (configurationAttempted && now - lastConfigurationPoll < CONFIG_POLL_INTERVAL_MS) return;
    configurationAttempted = true;
    lastConfigurationPoll = now;
    std::string response;
    if (!stationHttp.getConfiguration(response)) return;
    JsonDocument doc;
    if (deserializeJson(doc, response) || !doc["revision"].is<uint32_t>()) return;
    uint32_t revision = doc["revision"];
    if (revision > 2147483647U) return;
    if (!doc["interval_seconds"].isNull())
    {
        if (!doc["interval_seconds"].is<uint32_t>()) return;
        // A previous network command must not undo a newer local BLE setting.
        // A new server revision still takes precedence when it arrives.
        if (!station.settings.applyRemoteMeasurementConfiguration(doc["interval_seconds"].as<uint32_t>(), revision)) return;
    }
    const auto configuration = station.settings.getMeasurementConfiguration();
    revision = configuration.revision;
    const uint32_t seconds = configuration.seconds;
    JsonDocument report;
    report["revision"] = revision;
    report["interval_seconds"] = seconds;
    report["firmware_version"] = FIRMWARE_VERSION;
    const bool bleOverride = configuration.bleOverride;
    report["source"] = bleOverride ? "ble" : "remote";
    std::string payload;
    serializeJson(report, payload);
    // Repeated confirmation recovers a lost ACK; historical/offline timestamps stay intact.
    if (stationHttp.reportConfiguration(payload) && bleOverride)
        station.settings.confirmBleInterval(seconds, revision);
}
