#include <cassert>
#include <iostream>
#include "../src/location/location-schedule.h"
#include "../src/time/utc-epoch.h"

int main()
{
    LocationSchedule boot;
    assert(boot.due(0));
    boot.start(0, 10);
    assert(!boot.canAccept(10, 10, true)); // cached fix from before attempt
    assert(!boot.canAccept(10, 11, false)); // no fresh position
    assert(boot.canAccept(10, 11, true));
    boot.complete(10);
    assert(!boot.due(10 + LocationSchedule::DAILY_MS - 1));
    assert(boot.due(10 + LocationSchedule::DAILY_MS));

    boot.start(10 + LocationSchedule::DAILY_MS, 12);
    const uint64_t timeout = 10 + LocationSchedule::DAILY_MS + LocationSchedule::SEARCH_MS;
    assert(boot.expired(timeout));
    assert(!boot.canAccept(timeout, 13, true));
    boot.fail(timeout);
    assert(!boot.due(timeout + LocationSchedule::RETRY_MS - 1));
    assert(boot.due(timeout + LocationSchedule::RETRY_MS));

    // Long uptimes cross the 32-bit millis boundary without rescheduling early.
    LocationSchedule longUptime;
    const uint64_t nearWrap = 0xffffffffULL - 100;
    longUptime.start(nearWrap, 0xffffffffU);
    assert(longUptime.canAccept(nearWrap + 50, 0, true));
    longUptime.complete(nearWrap + 50);
    assert(!longUptime.due(nearWrap + 51));
    assert(longUptime.due(nearWrap + 50 + LocationSchedule::DAILY_MS));
    LocationSchedule reboot;
    assert(reboot.due(0)); // every restart starts a fresh search

    assert(gpsUtcEpoch(2024, 1, 1, 0, 0, 0) == 1704067200LL);
    assert(gpsUtcEpoch(2024, 2, 29, 0, 0, 0) == 1709164800LL);
    assert(gpsUtcEpoch(2026, 10, 5, 18, 0, 0) == 1791223200LL);
    assert(gpsUtcEpoch(2026, 2, 29, 0, 0, 0) == 0);
    assert(gpsUtcEpoch(2100, 2, 29, 0, 0, 0) == 0);
    assert(gpsUtcEpoch(2023, 1, 1, 0, 0, 0) == 0);
    std::cout << "GPS UTC and location scheduling checks passed\n";
}
