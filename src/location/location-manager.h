#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include "location-schedule.h"
#include "sensors/gnss-manager.h"
#include "time/time-manager.h"
#include "network/station-http-client.h"

struct StationLocation
{
    StationLocation() = default;
    StationLocation(double lat, double lon, int64_t at)
        : valid(true), latitude(lat), longitude(lon), acquiredAt(at) {}
    bool valid = false;
    double latitude = 0;
    double longitude = 0;
    int64_t acquiredAt = 0;
};

class LocationManager
{
public:
    void begin();
    void update(GNSSManager& gnss, TimeManager& clock, SettingsManager& settings,
                StationHttpClient& http);
    const StationLocation& getData() const { return location; }

private:
    bool persist();
    Preferences preferences;
    bool storageReady = false;
    bool dirty = false;
    bool pending = false;
    String deviceId;
    StationLocation location;
    LocationSchedule schedule;
    uint64_t nextSendAt = 0;
    uint64_t nextSaveAt = 0;
};
