#pragma once

#include <cstdint>

// Monotonic 64-bit milliseconds: independent of NTP changes and millis() wrap.
class LocationSchedule
{
public:
    static constexpr uint64_t DAILY_MS = 24ULL * 60 * 60 * 1000;
    static constexpr uint64_t SEARCH_MS = 5ULL * 60 * 1000;
    static constexpr uint64_t RETRY_MS = 15ULL * 60 * 1000;

    bool due(uint64_t now) const { return !searching && now >= nextAttempt; }
    void start(uint64_t now, uint32_t sequence)
    {
        searching = true;
        startedAt = now;
        initialSequence = sequence;
    }
    bool canAccept(uint64_t now, uint32_t sequence, bool fresh) const
    {
        return searching && now - startedAt < SEARCH_MS
            && sequence != initialSequence && fresh;
    }
    bool expired(uint64_t now) const
    {
        return searching && now - startedAt >= SEARCH_MS;
    }
    void complete(uint64_t now)
    {
        searching = false;
        nextAttempt = now + DAILY_MS;
    }
    void fail(uint64_t now)
    {
        searching = false;
        nextAttempt = now + RETRY_MS;
    }

private:
    bool searching = false;
    uint64_t startedAt = 0;
    uint64_t nextAttempt = 0;
    uint32_t initialSequence = 0;
};
