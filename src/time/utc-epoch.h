#pragma once

#include <cstdint>

// GPS calendar -> Unix UTC, without relying on the system clock or time zone.
inline int64_t gpsUtcEpoch(int year, int month, int day, int hour, int minute, int second)
{
    if (year < 2024 || year > 2100 || month < 1 || month > 12
        || hour < 0 || hour > 23 || minute < 0 || minute > 59
        || second < 0 || second > 59) return 0;
    const auto leap = [](int y) { return y % 4 == 0 && (y % 100 != 0 || y % 400 == 0); };
    const int months[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    const int maxDay = months[month - 1] + (month == 2 && leap(year) ? 1 : 0);
    if (day < 1 || day > maxDay) return 0;
    int64_t days = 0;
    for (int y = 1970; y < year; ++y) days += leap(y) ? 366 : 365;
    for (int m = 1; m < month; ++m) days += months[m - 1] + (m == 2 && leap(year) ? 1 : 0);
    days += day - 1;
    return days * 86400 + hour * 3600 + minute * 60 + second;
}
