#pragma once

#include <Arduino.h>
#include <cstdint>

class TimeManager
{
public:
    void begin();

    bool waitForSynchronization(
        unsigned long timeoutMs = 10000
    );

    void update();

    bool isSynchronized() const;

    int64_t now() const;

private:
    void requestSynchronization();

    bool hasValidSystemTime() const;

    bool synchronized = false;

    unsigned long lastAttemptMs = 0;

    static constexpr int64_t
        MIN_VALID_EPOCH =
            1704067200LL;
};