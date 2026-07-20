#include "wifi-manager.h"
#include <WiFi.h>

void WiFiManager::begin(const char* ssid, const char* password)
{
    _ssid = ssid;
    _password = password;

    connect();
}

void WiFiManager::connect()
{
    Serial.println();
    Serial.println("Conectando ao WiFi...");

    WiFi.mode(WIFI_STA);
    WiFi.begin(_ssid, _password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.println("WiFi conectado!");

    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
}

bool WiFiManager::isConnected()
{
    return WiFi.status() == WL_CONNECTED;
}

String WiFiManager::getSSID() const
{
    return WiFi.SSID();
}

String WiFiManager::getIP() const
{
    return WiFi.localIP().toString();
}