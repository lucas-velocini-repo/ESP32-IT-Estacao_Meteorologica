#include "station-context.h"

#include <ArduinoJson.h>
#include <esp_system.h>
#include "api/json-builder.h"
#include "config/config.h"
#include "power/power-manager.h"

void StationContext::begin(bool batteryMode)
{
    settings.begin();
    location.begin();
    pendingMeasurements.begin();
    undatedMeasurements.begin();
    bootId = esp_random();
    sensors.begin(Pins::I2C_SDA, Pins::I2C_SCL, batteryMode);
}

StationData StationContext::measurement(const SensorData& data)
{
    StationData station;
    station.deviceId = settings.getDeviceId();
    station.deviceName = settings.getStationName();
    station.measuredAt = clock.now(); // Apenas RTC; não solicita NTP.
    station.sensors = data;
    const StationLocation& lastLocation = location.getData();
    station.locationValid = lastLocation.valid && location.belongsTo(station.deviceId);
    station.latitude = lastLocation.latitude;
    station.longitude = lastLocation.longitude;
    return station;
}

bool StationContext::saveBatteryMeasurement(const SensorData& data)
{
    const StationData station = measurement(data);
    std::string payload = JsonBuilder::buildStationPayload(station);
    if (station.measuredAt > 0 && !station.deviceId.isEmpty())
        return pendingMeasurements.enqueue(payload);

    // Boot só na bateria: mantém a coleta sem inventar UTC ou fazer acesso GNSS.
    JsonDocument doc;
    if (deserializeJson(doc, payload)) return false;
    doc["timestamp"] = nullptr;
    doc["boot_id"] = bootId;
    doc["uptime_ms"] = PowerManager::nowMs();
    doc["reason"] = station.deviceId.isEmpty() ? "identity_missing" : "clock_unavailable";
    payload.clear();
    serializeJson(doc, payload);
    return undatedMeasurements.enqueue(payload);
}
