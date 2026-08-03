#include "time-manager.h"

#include <WiFi.h>
#include <time.h>

namespace
{
    constexpr unsigned long
        RETRY_INTERVAL_MS =
            60UL * 1000UL;

    const char* NTP_SERVER_1 =
        "pool.ntp.org";

    const char* NTP_SERVER_2 =
        "time.google.com";

    const char* NTP_SERVER_3 =
        "time.cloudflare.com";
}

void TimeManager::begin()
{
    requestSynchronization();
}

void TimeManager::requestSynchronization()
{
    Serial.println();
    Serial.println(
        "[Time] Solicitando sincronização NTP..."
    );

    // UTC: deslocamento e horário de verão iguais a zero.
    configTime(
        0,
        0,
        NTP_SERVER_1,
        NTP_SERVER_2,
        NTP_SERVER_3
    );

    lastAttemptMs =
        millis();
}

bool TimeManager::waitForSynchronization(
    unsigned long timeoutMs
)
{
    if(
        WiFi.status() != WL_CONNECTED
    )
    {
        Serial.println(
            "[Time] WiFi desconectado."
        );

        return false;
    }

    unsigned long start =
        millis();

    while(
        millis() - start < timeoutMs
    )
    {
        if(hasValidSystemTime())
        {
            synchronized = true;

            Serial.print(
                "[Time] Sincronizado. Epoch: "
            );

            Serial.println(
                static_cast<long long>(
                    now()
                )
            );

            return true;
        }

        delay(200);
    }

    Serial.println(
        "[Time] NTP não respondeu dentro do prazo."
    );

    return false;
}

void TimeManager::update()
{
    if(hasValidSystemTime())
    {
        synchronized = true;
        return;
    }

    synchronized = false;

    if(
        WiFi.status() != WL_CONNECTED
    )
    {
        return;
    }

    if(
        millis() - lastAttemptMs >=
        RETRY_INTERVAL_MS
    )
    {
        requestSynchronization();
    }
}

bool TimeManager::hasValidSystemTime() const
{
    time_t currentTime;

    time(
        &currentTime
    );

    return (
        static_cast<int64_t>(
            currentTime
        )
        >= MIN_VALID_EPOCH
    );
}

bool TimeManager::isSynchronized() const
{
    return hasValidSystemTime();
}

int64_t TimeManager::now() const
{
    if(!hasValidSystemTime())
    {
        return 0;
    }

    time_t currentTime;

    time(
        &currentTime
    );

    return static_cast<int64_t>(
        currentTime
    );
}