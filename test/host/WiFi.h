#pragma once
constexpr int WL_CONNECTED = 3;
extern int wifiStatus;
struct WiFiStub { int status() const { return wifiStatus; } };
extern WiFiStub WiFi;
