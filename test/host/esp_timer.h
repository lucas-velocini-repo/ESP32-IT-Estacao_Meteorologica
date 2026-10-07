#pragma once
#include <cstdint>
extern uint64_t uptimeMs;
inline int64_t esp_timer_get_time() { return uptimeMs * 1000; }
