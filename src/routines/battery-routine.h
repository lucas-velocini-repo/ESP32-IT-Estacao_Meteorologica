#pragma once

#include <cstdint>

#include "station/station-context.h"

class BatteryRoutine
{
public:
    explicit BatteryRoutine(StationContext& context) : station(context) {}
    void begin();
    void update();
    void end();

private:
    enum class Phase { Sleeping, Warming, Reading };
    StationContext& station;
    Phase phase = Phase::Sleeping;
    uint64_t startedAt = 0;
    uint64_t nextActionAt = 0;
    uint64_t nextMeasurementAt = 0;
    uint64_t readDeadline = 0;
};
