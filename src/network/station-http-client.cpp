#include "station-http-client.h"

#include <HTTPClient.h>
#include <WiFi.h>

void StationHttpClient::begin(
    SettingsManager& settingsManager
)
{
    settings = &settingsManager;
}

bool StationHttpClient::send(
    const std::string& payload
)
{
    if(settings == nullptr)
    {
        Serial.println(
            "[HTTP] Cliente não inicializado."
        );

        return false;
    }

    if(WiFi.status() != WL_CONNECTED)
    {
        Serial.println(
            "[HTTP] WiFi não conectado."
        );

        return false;
    }

    String serverUrl =
        settings->getServer();

    serverUrl.trim();

    if(serverUrl.length() == 0)
    {
        Serial.println(
            "[HTTP] URL do servidor não configurada."
        );

        return false;
    }

    HTTPClient http;

    Serial.println();
    Serial.println("[HTTP] Enviando dados...");
    Serial.print("[HTTP] URL: ");
    Serial.println(serverUrl);

    if(!http.begin(serverUrl))
    {
        Serial.println(
            "[HTTP] Não foi possível iniciar a conexão."
        );

        return false;
    }

    http.addHeader(
        "Content-Type",
        "application/json"
    );

    String requestBody(payload.c_str());

    int httpCode = http.POST(requestBody);

    if(httpCode <= 0)
    {
        Serial.print(
            "[HTTP] Falha no POST: "
        );

        Serial.println(
            HTTPClient::errorToString(
                httpCode
            )
        );

        http.end();

        return false;
    }

    Serial.print(
        "[HTTP] Código de resposta: "
    );

    Serial.println(httpCode);

    String response =
        http.getString();

    if(response.length() > 0)
    {
        Serial.print(
            "[HTTP] Resposta: "
        );

        Serial.println(response);
    }

    bool success =
        httpCode >= 200 &&
        httpCode < 300;

    if(success)
    {
        Serial.println(
            "[HTTP] Dados enviados com sucesso."
        );
    }
    else
    {
        Serial.println(
            "[HTTP] Servidor rejeitou os dados."
        );
    }

    http.end();

    return success;
}