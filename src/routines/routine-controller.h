#pragma once

#include <cstdint>

#include "station/station-context.h"
#include "normal-routine.h"
#include "battery-routine.h"

class RoutineController
{
public:
    explicit RoutineController(bool testMode)
        : testMode(testMode), normal(station), battery(station) {}
    void begin();
    void update();

private:
    enum class Mode { Test, Normal, Battery };
    const bool testMode;
    StationContext station;
    NormalRoutine normal;
    BatteryRoutine battery;
    Mode mode = Mode::Battery;
    bool observedPower = false;
    uint64_t powerChangedAt = 0;
};
