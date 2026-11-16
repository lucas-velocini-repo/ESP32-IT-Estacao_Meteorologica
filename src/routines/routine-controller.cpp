#include "routine-controller.h"

#include <Arduino.h>
#include "config/config.h"
#include "hardware-test.h"
#include "led/status-leds.h"
#include "power/power-manager.h"

void RoutineController::begin()
{
    Serial.begin(19200);
    Serial.printf("[Firmware] Versao: %s\n", FIRMWARE_VERSION);
    PowerManager::begin();
    StatusLeds::begin();
    if (testMode)
    {
        mode = Mode::Test;
        PowerManager::gnssPower(true);
        HardwareTest::begin(station.sensors, station.gnss);
        return;
    }

    observedPower = PowerManager::externalPower();
    mode = observedPower ? Mode::Normal : Mode::Battery;
    station.begin(!observedPower);
    if (observedPower) normal.begin();
    else battery.begin();
}

void RoutineController::update()
{
    if (mode == Mode::Test)
    {
        HardwareTest::update(station.sensors, station.gnss);
        delay(1);
        return;
    }

    const bool powered = PowerManager::externalPower();
    const uint64_t now = PowerManager::nowMs();
    if (powered != observedPower)
    {
        observedPower = powered;
        powerChangedAt = now;
    }
    const Mode requested = powered ? Mode::Normal : Mode::Battery;
    if (requested != mode)
    {
        if (now - powerChangedAt < BatteryConfig::POWER_DEBOUNCE_MS)
        {
            delay(1);
            return;
        }
        if (mode == Mode::Normal) normal.end();
        else battery.end();
        mode = requested;
        if (mode == Mode::Normal) normal.begin();
        else battery.begin();
    }
    if (mode == Mode::Normal) normal.update();
    else battery.update();
}
