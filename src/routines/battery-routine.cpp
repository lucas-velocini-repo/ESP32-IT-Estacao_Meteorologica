#include "battery-routine.h"

#include <algorithm>
#include "config/config.h"
#include "led/status-leds.h"
#include "power/power-manager.h"

void BatteryRoutine::begin()
{
    PowerManager::gnssPower(false);
    PowerManager::batteryCpu();
    station.sensors.setBatteryMode(true);
    startedAt = PowerManager::nowMs();
    nextActionAt = startedAt;
    phase = Phase::Sleeping;
    Serial.println("[Battery] Sem WiFi, BLE, NTP ou leitura GNSS. Coletas locais.");
    if (Pins::GNSS_POWER_ENABLE < 0)
        Serial.println("[Battery] GPS sem EN configurado: corte físico exige load switch.");
}

void BatteryRoutine::end()
{
    station.sensors.suspendBatteryMeasurement();
    StatusLeds::allOff();
}

void BatteryRoutine::update()
{
    if (PowerManager::externalPower()) return;
    uint64_t now = PowerManager::nowMs();
    StatusLeds::battery(now - startedAt);

    if (now >= nextActionAt)
    {
        if (phase == Phase::Sleeping)
        {
            const uint64_t interval = station.settings.getMeasurementIntervalSeconds() * 1000ULL;
            nextMeasurementAt = now + interval;
            const bool started = station.sensors.prepareBatteryMeasurement();
            nextActionAt = PowerManager::nowMs() + (started ? BatteryConfig::SPS30_WARMUP_MS : 0);
            phase = Phase::Warming;
        }
        else if (phase == Phase::Warming)
        {
            station.sensors.prepareBatteryLight();
            nextActionAt = PowerManager::nowMs() + BatteryConfig::LIGHT_CONVERSION_MS;
            readDeadline = nextActionAt + BatteryConfig::SPS30_READ_TIMEOUT_MS;
            phase = Phase::Reading;
        }
        else
        {
            SensorData data;
            if (station.sensors.readBattery(data, now >= readDeadline))
            {
                station.sensors.suspendBatteryMeasurement();
                if (!station.saveBatteryMeasurement(data))
                    Serial.println("[Battery] ERRO ao salvar coleta local.");
                // Um intervalo menor que a estabilização não reinicia o ventilador
                // em sequência: garante pelo menos 30 s de repouso entre ciclos.
                const uint64_t after = PowerManager::nowMs();
                nextActionAt = std::max(nextMeasurementAt,
                    after + static_cast<uint64_t>(BatteryConfig::MIN_SLEEP_MS));
                phase = Phase::Sleeping;
            }
            else nextActionAt = std::min(readDeadline, now + 1000);
        }
    }

    now = PowerManager::nowMs();
    const uint64_t phaseMs = (now - startedAt) % BatteryConfig::LED_PERIOD_MS;
    const bool ledOn = StatusLeds::battery(now - startedAt);
    const uint64_t untilLed = ledOn ? BatteryConfig::LED_ON_MS - phaseMs
        : BatteryConfig::LED_PERIOD_MS - phaseMs;
    const uint64_t untilAction = nextActionAt > now ? nextActionAt - now : 0;
    PowerManager::idleBattery(static_cast<uint32_t>(std::min(untilLed, untilAction)), ledOn);
}
