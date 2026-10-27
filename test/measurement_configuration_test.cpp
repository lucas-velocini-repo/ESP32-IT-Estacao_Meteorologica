#include <cassert>
#include <iostream>
#include "config/settings-manager.h"

SerialStub Serial;
bool storageFails = false;
std::map<std::string, std::string> nvs;

int main()
{
    SettingsManager initial;
    initial.begin();
    assert(initial.getMeasurementIntervalSeconds() == 300);
    assert(initial.getConfigurationRevision() == 0);
    assert(!initial.saveMeasurementConfiguration(29, 1));
    assert(!initial.saveMeasurementConfiguration(86401, 1));
    assert(initial.saveMeasurementConfiguration(30, 1));
    assert(initial.getMeasurementIntervalSeconds() == 30);
    SettingsManager reboot;
    reboot.begin();
    assert(reboot.getMeasurementIntervalSeconds() == 30);
    assert(reboot.getConfigurationRevision() == 1);
    storageFails = true;
    // An unchanged configuration requires no write; failed storage never acknowledges a new command.
    assert(reboot.saveMeasurementConfiguration(30, 1));
    assert(!reboot.saveMeasurementConfiguration(600, 2));
    assert(reboot.getMeasurementIntervalSeconds() == 30);
    assert(reboot.getConfigurationRevision() == 1);
    storageFails = false;
    assert(reboot.saveMeasurementConfiguration(86400, 2));
    SettingsManager next;
    next.begin();
    assert(next.getMeasurementIntervalSeconds() == 86400);
    assert(next.getConfigurationRevision() == 2);
    nvs["interval_cfg"] = R"({"seconds":-1,"revision":99})";
    SettingsManager corrupt;
    corrupt.begin();
    assert(corrupt.getMeasurementIntervalSeconds() == 300);
    std::cout << "Measurement configuration persistence tests passed\n";
}
