#pragma once

#include <Arduino.h>

class WiFiManager
{
public:
    void begin(const char* ssid, const char* password);
    bool isConnected();
    bool isConnected() const;
    String getSSID() const;
    String getIP() const;

private:
    void connect();
    const char* _ssid;
    const char* _password;
};