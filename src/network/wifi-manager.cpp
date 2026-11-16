#include "wifi-manager.h"
#include <WiFi.h>
#include "power/power-manager.h"

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

    if (!PowerManager::externalPower()) return;
    WiFi.setAutoReconnect(true);
    WiFi.mode(WIFI_STA);
    WiFi.begin(
        _ssid.c_str(),
        _password.c_str()
    );

    unsigned long start = millis();

    while (PowerManager::externalPower() && WiFi.status() != WL_CONNECTED &&
        millis() - start < 10000)
    {
        delay(20);

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

    if (!PowerManager::externalPower()) return false;
    WiFi.disconnect();

    delay(20);

    if (!PowerManager::externalPower()) return false;
    WiFi.setAutoReconnect(true);
    WiFi.mode(WIFI_STA);

    WiFi.begin(
        _ssid.c_str(),
        _password.c_str()
    );

    Serial.print(
        "[WiFi] Conectando"
    );

    if (!PowerManager::externalPower()) return false;

    const unsigned long timeoutMs =
        30000;

    unsigned long start =
        millis();

    while(
        PowerManager::externalPower() && WiFi.status() != WL_CONNECTED &&
        millis() - start < timeoutMs
    )
    {
        delay(20);

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
void WiFiManager::end()
{
    WiFi.setAutoReconnect(false);
    WiFi.disconnect(true, false); // Não apaga as credenciais persistidas.
    WiFi.mode(WIFI_OFF);
    lastReconnectAttempt = 0;
    reconnectIntervalMs = 5000;
}
