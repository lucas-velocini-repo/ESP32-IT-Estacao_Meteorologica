#pragma once

#include "station/station-context.h"
#include "network/wifi-manager.h"
#include "network/station-http-client.h"
#include "bluetooth/ble-manager.h"
#include "protocol/protocol-handler.h"

class NormalRoutine
{
public:
    explicit NormalRoutine(StationContext& context) : station(context) {}
    void begin();
    void update();
    void end();

private:
    StationContext& station;
    WiFiManager wifi;
    BLEManager ble;
    ProtocolHandler protocol;
    StationHttpClient stationHttp;
    unsigned long lastPendingRetryTime = 0;
    unsigned long lastDataTime = 0;
    unsigned long lastConfigurationPoll = 0;
    bool configurationAttempted = false;

    void updateStatusLeds();
    void processPendingWifiConfiguration();
    void processPendingMeasurements();
    void sendStationData();
    void synchronizeMeasurementConfiguration();
};
