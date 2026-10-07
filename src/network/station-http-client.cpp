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
    return request(payload, "POST", false);
}

bool StationHttpClient::sendLocation(const std::string& payload)
{
    return request(payload, "PATCH", true);
}

bool StationHttpClient::request(const std::string& payload, const char* method, bool location)
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

    if (location)
    {
        while (serverUrl.endsWith("/")) serverUrl.remove(serverUrl.length() - 1);
        if (!serverUrl.endsWith("/measurements"))
        {
            Serial.println("[Location] URL deve terminar em /measurements.");
            return false;
        }
        serverUrl.remove(serverUrl.length() - String("/measurements").length());
        serverUrl += "/devices/location";
    }

    HTTPClient http;
    if (location)
    {
        http.setConnectTimeout(3000);
        http.setTimeout(3000);
    }

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

    String apiToken =
        settings->getApiToken();

    apiToken.trim();

    if(apiToken.length() == 0)
    {
        Serial.println(
            "[HTTP] Token de autenticação não configurado."
        );

        http.end();

        return false;
    }

    String authorization =
        "Bearer " + apiToken;

    http.addHeader(
        "Authorization",
        authorization
    );

    String requestBody(payload.c_str());

    int httpCode = http.sendRequest(method, requestBody);

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
