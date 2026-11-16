#pragma once
#include <string>
#include <cstddef>
class String: public std::string {
public:
    using std::string::string;
    String(const std::string& value):std::string(value) {}
    bool endsWith(const char* suffix) const {
        const std::string s(suffix);
        return size()>=s.size() && compare(size()-s.size(),s.size(),s)==0;
    }
    void remove(size_t from) { erase(from); }
};
struct SerialStub {
    void println() {}
    template<class T> void println(const T&) {}
    template<class T> void print(const T&) {}
};
extern SerialStub Serial;
