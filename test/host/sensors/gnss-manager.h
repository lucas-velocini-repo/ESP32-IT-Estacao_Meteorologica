#pragma once
#include "Arduino.h"
struct GNSSData {
    bool valid = false;
    double latitude = 0, longitude = 0;
    unsigned long lastFixAgeMs = 0;
    uint32_t sequence = 0;
    int64_t acquiredAt = 0;
};
struct GNSSManager {
    GNSSData data;
    GNSSData getData() const { return data; }
};
