#pragma once
#include <cstdint>
struct TimeManager {
    int64_t epoch = 0;
    bool isSynchronized() const { return epoch != 0; }
    int64_t now() const { return epoch; }
};
