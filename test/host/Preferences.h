#pragma once
#include "Arduino.h"
#include <map>
extern std::map<std::string, std::string> nvs;
extern bool storageFails;
class Preferences {
public:
    bool isKey(const char* key) { return nvs.count(key) != 0; }
    bool begin(const char*, bool) { return !storageFails; }
    String getString(const char* key, const char* fallback) {
        return nvs.count(key) ? nvs[key] : fallback;
    }
    size_t putString(const char* key, const String& value) {
        if (storageFails) return 0;
        nvs[key] = value;
        return value.length();
    }
};
