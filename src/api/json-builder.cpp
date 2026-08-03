#include "json-builder.h"

#include <ArduinoJson.h>

std::string JsonBuilder::buildStationPayload(
    const StationData& station
)
{
    JsonDocument doc;

    doc["device_id"] = station.deviceId;
    doc["device_name"] = station.deviceName;
    doc["measured_at"] = station.measuredAt;

    JsonObject location =
        doc["location"].to<JsonObject>();

    location["lat"] = station.latitude;
    location["lon"] = station.longitude;

    JsonObject pm =
        doc["pm"].to<JsonObject>();

    pm["1.0"] = station.sensors.pm1_0;
    pm["2.5"] = station.sensors.pm2_5;
    pm["4.0"] = station.sensors.pm4_0;
    pm["10.0"] = station.sensors.pm10_0;

    JsonObject nc =
        doc["nc"].to<JsonObject>();

    nc["0.5"] = station.sensors.nc0_5;
    nc["1.0"] = station.sensors.nc1_0;
    nc["2.5"] = station.sensors.nc2_5;
    nc["4.0"] = station.sensors.nc4_0;
    nc["10.0"] = station.sensors.nc10_0;

    doc["typical_size"] =
        station.sensors.typicalSize;

    doc["light"] =
        station.sensors.light;

    doc["temperature"] =
        station.sensors.temperature;

    doc["humidity"] =
        station.sensors.humidity;

    doc["pressure"] =
        station.sensors.pressure;

    std::string json;

    serializeJson(doc, json);

    return json;
}