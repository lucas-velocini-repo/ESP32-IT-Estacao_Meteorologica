#include "location-manager.h"

#include <ArduinoJson.h>
#include <WiFi.h>
#include <esp_timer.h>
#include <cmath>
#include <limits>

namespace
{
    constexpr uint64_t SEND_RETRY_MS = 60ULL * 1000;
    constexpr unsigned long FRESH_FIX_MS = 5000;
}

void LocationManager::begin()
{
    storageReady = preferences.begin("location", false);
    if (!storageReady)
    {
        Serial.println("[Location] Falha ao abrir armazenamento NVS.");
        return;
    }
    JsonDocument doc;
    if (deserializeJson(doc, preferences.getString("state", ""))) return;
    const double lat = doc["latitude"] | std::numeric_limits<double>::quiet_NaN();
    const double lon = doc["longitude"] | std::numeric_limits<double>::quiet_NaN();
    const int64_t epoch = doc["acquired_at"] | int64_t(0);
    if (!std::isfinite(lat) || !std::isfinite(lon) || lat < -90 || lat > 90
        || lon < -180 || lon > 180 || epoch < 1704067200LL) return;
    deviceId = doc["device_id"].as<String>();
    if (deviceId.isEmpty()) return;
    location = StationLocation(lat, lon, epoch);
    pending = doc["pending"] | false;
    Serial.println("[Location] Última localização recuperada.");
    // The schedule always starts a new search after boot, even with a saved fix.
}

bool LocationManager::persist()
{
    if (!storageReady) storageReady = preferences.begin("location", false);
    if (!storageReady) return false;
    JsonDocument doc;
    doc["device_id"] = deviceId;
    doc["latitude"] = location.latitude;
    doc["longitude"] = location.longitude;
    doc["acquired_at"] = location.acquiredAt;
    doc["pending"] = pending;
    String record;
    serializeJson(doc, record);
    // A single NVS value prevents partial coordinate/timestamp/pending updates.
    return preferences.putString("state", record) == record.length();
}

void LocationManager::update(GNSSManager& gnss, TimeManager& clock,
    SettingsManager& settings, StationHttpClient& http)
{
    const uint64_t now = static_cast<uint64_t>(esp_timer_get_time()) / 1000;
    const String configuredId = settings.getDeviceId();
    if (configuredId.isEmpty()) return;
    if (deviceId != configuredId)
    {
        // Never send a cached position belonging to a previous registration.
        deviceId = configuredId;
        location = {};
        pending = false;
        dirty = true;
        nextSaveAt = 0;
        nextSendAt = 0;
        schedule = LocationSchedule();
    }
    const GNSSData fix = gnss.getData();
    if (schedule.due(now))
    {
        schedule.start(now, fix.sequence);
        Serial.println("[Location] Buscando nova posição (até 5 minutos).");
    }
    const bool fresh = fix.valid && fix.lastFixAgeMs <= FRESH_FIX_MS
        && std::isfinite(fix.latitude) && std::isfinite(fix.longitude)
        && fix.latitude >= -90 && fix.latitude <= 90
        && fix.longitude >= -180 && fix.longitude <= 180;
    if (schedule.canAccept(now, fix.sequence, fresh))
    {
        int64_t acquiredAt = fix.acquiredAt;
        if (acquiredAt == 0 && clock.isSynchronized())
            acquiredAt = clock.now() - fix.lastFixAgeMs / 1000;
        // Prefer GPS UTC, so an offline boot can save a correctly dated fix.
        // Keep searching if neither GPS UTC nor the system clock is available.
        if (acquiredAt >= 1704067200LL
            && (!location.valid || acquiredAt > location.acquiredAt))
        {
            location = StationLocation(fix.latitude, fix.longitude, acquiredAt);
            pending = true;
            dirty = true;
            nextSaveAt = 0;
            nextSendAt = 0;
            schedule.complete(now);
            Serial.println("[Location] Nova posição obtida. Próxima busca em 24 horas.");
        }
    }
    if (schedule.expired(now))
    {
        schedule.fail(now);
        Serial.println("[Location] Sem nova posição. Nova tentativa em 15 minutos.");
    }
    if (dirty && now >= nextSaveAt)
    {
        dirty = !persist();
        nextSaveAt = now + SEND_RETRY_MS;
        if (dirty) Serial.println("[Location] Falha ao persistir. Tentaremos novamente.");
    }
    if (!pending || dirty || now < nextSendAt || WiFi.status() != WL_CONNECTED
        || settings.getApiToken().isEmpty()) return;
    nextSendAt = now + SEND_RETRY_MS;
    JsonDocument payload;
    payload["latitude"] = location.latitude;
    payload["longitude"] = location.longitude;
    // Pydantic accepts Unix UTC seconds for the aware datetime field.
    payload["acquired_at"] = location.acquiredAt;
    std::string json;
    serializeJson(payload, json);
    if (http.sendLocation(json))
    {
        pending = false;
        dirty = !persist();
        nextSaveAt = now + SEND_RETRY_MS;
        Serial.println("[Location] Localização confirmada pelo servidor.");
    }
}
