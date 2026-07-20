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

bool WiFiManager::isConnected() const
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

bool WiFiManager::configure(
    const char* ssid,
    const char* password
)
{
    _ssid = ssid;
    _password = password;

    WiFi.disconnect(true);

    WiFi.begin(_ssid, _password);

    Serial.println("Conectando...");

    unsigned long start = millis();

    while(
        WiFi.status() != WL_CONNECTED &&
        millis() - start < 15000
    )
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println();

    return WiFi.status() == WL_CONNECTED;
}