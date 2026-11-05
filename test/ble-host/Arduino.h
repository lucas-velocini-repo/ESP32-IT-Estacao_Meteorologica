#pragma once
#include <cstdint>
#include <string>
#include <cstddef>
class String : public std::string {
public:
    using std::string::string;
    String(const std::string& value) : std::string(value) {}
    String substring(size_t from) const { return substr(from); }
};
inline String operator+(const char* left, const String& right) {
    return String(std::string(left) + static_cast<const std::string&>(right));
}
struct SerialStub {
    void println(const char*) {}
    template<typename... Args> void printf(const char*, Args...) {}
};
extern SerialStub Serial;
extern unsigned long fakeMillis;
extern unsigned delayCalls;
inline unsigned long millis() { return fakeMillis; }
inline void delay(unsigned long ms) { fakeMillis += ms; ++delayCalls; }
