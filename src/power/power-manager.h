#pragma once

#include <cstdint>

namespace PowerManager
{
    void begin();
    bool externalPower();
    void internalLedsOff();
    void gnssPower(bool on);
    void batteryCpu();
    void normalCpu();
    uint64_t nowMs();
    void idleBattery(uint32_t milliseconds, bool redLedOn);
}
