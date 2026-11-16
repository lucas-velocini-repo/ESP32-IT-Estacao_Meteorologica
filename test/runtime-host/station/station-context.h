#pragma once
#include <vector>
#include "sensors/sensor-data.h"
struct RuntimeCalls {
    int contextBegins=0, normalBegins=0, normalUpdates=0, normalEnds=0;
    int testBegins=0, testUpdates=0, starts=0, suspends=0, lightStarts=0;
    bool initialBattery=false, pmAvailable=true;
    unsigned intervalSeconds=300;
    std::vector<SensorData> saved;
};
extern RuntimeCalls calls;
struct SensorManager {
    void setBatteryMode(bool) {}
    bool prepareBatteryMeasurement() { ++calls.starts; return true; }
    void prepareBatteryLight() { ++calls.lightStarts; }
    bool readBattery(SensorData& data,bool partial) {
        if (!calls.pmAvailable && !partial) return false;
        data.temperature=25;
        if(calls.pmAvailable) data.pm2_5=12;
        return true;
    }
    void suspendBatteryMeasurement() { ++calls.suspends; }
};
struct GNSSManager {};
struct SettingsManager { unsigned getMeasurementIntervalSeconds() { return calls.intervalSeconds; } };
struct StationContext {
    SensorManager sensors;
    GNSSManager gnss;
    SettingsManager settings;
    void begin(bool battery) { ++calls.contextBegins; calls.initialBattery=battery; }
    bool saveBatteryMeasurement(const SensorData& data) { calls.saved.push_back(data); return true; }
};
