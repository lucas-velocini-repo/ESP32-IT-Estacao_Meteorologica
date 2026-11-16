#pragma once

#include "config/settings-manager.h"
#include "sensors/sensor-manager.h"
#include "sensors/gnss-manager.h"
#include "storage/pending-measurement-store.h"
#include "location/location-manager.h"
#include "time/time-manager.h"
#include "station-data.h"

// Serviços locais compartilhados. Nenhuma inicialização de rádio aqui.
struct StationContext
{
    SettingsManager settings;
    SensorManager sensors;
    GNSSManager gnss;
    PendingMeasurementStore pendingMeasurements;
    PendingMeasurementStore undatedMeasurements{"/undated.jsonl", "/undated.tmp"};
    LocationManager location;
    TimeManager clock;

    void begin(bool batteryMode);
    StationData measurement(const SensorData& data);
    bool saveBatteryMeasurement(const SensorData& data);

private:
    uint32_t bootId = 0;
};
