#pragma once
#include "station/station-context.h"
namespace HardwareTest {
inline void begin(SensorManager&,GNSSManager&) { ++calls.testBegins; }
inline void update(SensorManager&,GNSSManager&) { ++calls.testUpdates; }
}
