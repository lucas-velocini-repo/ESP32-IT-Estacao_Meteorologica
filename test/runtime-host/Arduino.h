#pragma once
#include <cstdint>
#include <map>
constexpr int LOW=0, HIGH=1, INPUT=0, OUTPUT=1;
struct SerialStub {
    void begin(unsigned) {}
    void println(const char*) {}
    template<class... Args> void printf(const char*, Args...) {}
};
extern SerialStub Serial;
extern uint64_t fakeNow;
extern std::map<uint8_t,int> pwm;
inline void analogWrite(uint8_t pin,int value) { pwm[pin]=value; }
inline void digitalWrite(uint8_t,int) {}
inline void pinMode(uint8_t,int) {}
inline void delay(unsigned long ms) { fakeNow += ms; }
