#pragma once

#include <Arduino.h>

class WiFiManager
{
public:
    void begin(const char* ssid, const char* password);
    bool isConnected() const;
    bool configure(
        const char* ssid,
        const char* password
    );
    void update();
    String getSSID() const;
    String getIP() const;

private:
    void connect();

    unsigned long lastReconnectAttempt = 0;
    unsigned long reconnectIntervalMs = 5000;
    static constexpr unsigned long MAX_RECONNECT_INTERVAL_MS = 60000;
    
    String _ssid;
    String _password;
};