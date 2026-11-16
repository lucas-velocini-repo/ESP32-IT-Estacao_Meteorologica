#pragma once
#include <cstdint>
#include <cstddef>
struct SerialStub {
    template<class... Args> void printf(const char*,Args...) {}
};
extern SerialStub Serial;
inline void delay(unsigned long) {}
