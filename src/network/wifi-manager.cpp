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
    WiFi.begin(
        _ssid.c_str(),
        _password.c_str()
    );

    unsigned long start = millis();

    while (WiFi.status() != WL_CONNECTED &&
        millis() - start < 10000)
    {
        delay(500);
        Serial.print(".");
    }

    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println();
        Serial.println("Falha ao conectar ao WiFi.");
        return;
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

    Serial.println();
    Serial.println(
        "[WiFi] Aplicando nova configuração..."
    );

    WiFi.disconnect();

    delay(250);

    WiFi.mode(WIFI_STA);

    WiFi.begin(
        _ssid.c_str(),
        _password.c_str()
    );

    Serial.print(
        "[WiFi] Conectando"
    );

    const unsigned long timeoutMs =
        30000;

    unsigned long start =
        millis();

    while(
        WiFi.status() != WL_CONNECTED &&
        millis() - start < timeoutMs
    )
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println();

    if(
        WiFi.status()
        != WL_CONNECTED
    )
    {
        Serial.println(
            "[WiFi] Timeout ao conectar."
        );

        return false;
    }

    Serial.println(
        "[WiFi] Conectado com sucesso."
    );

    Serial.print(
        "[WiFi] Rede: "
    );

    Serial.println(
        WiFi.SSID()
    );

    Serial.print(
        "[WiFi] IP: "
    );

    Serial.println(
        WiFi.localIP()
    );

    return true;
}

void WiFiManager::update()
{
    if(WiFi.status() == WL_CONNECTED)
    {
        reconnectIntervalMs = 5000;
        return;
    }

    if(_ssid.length() == 0)
    {
        return;
    }

    unsigned long now =
        millis();

    if(
        now - lastReconnectAttempt
        < reconnectIntervalMs
    )
    {
        return;
    }

    lastReconnectAttempt =
        now;

    Serial.print(
        "[WiFi] Reconectando à rede: "
    );

    Serial.println(
        _ssid
    );

    WiFi.disconnect();

    delay(50);

    WiFi.mode(WIFI_STA);

    WiFi.begin(
        _ssid.c_str(),
        _password.c_str()
    );

    reconnectIntervalMs *= 2;

    if(
        reconnectIntervalMs
        > MAX_RECONNECT_INTERVAL_MS
    )
    {
        reconnectIntervalMs =
            MAX_RECONNECT_INTERVAL_MS;
    }
}