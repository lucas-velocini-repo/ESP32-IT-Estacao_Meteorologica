#include <cassert>
#include <cmath>
#include <iostream>
#include "sensors/sps30.h"
#include <Arduino.h>
#include <Wire.h>
SerialStub Serial;
WireStub Wire;
std::vector<std::string> commands;
bool sampleReady=false;
int stopError=0,sleepError=0,wakeError=0,startError=0,readError=0;
int main() {
    SPS30 sensor;
    sensor.begin(true);
    assert((commands==std::vector<std::string>{"bind","wake","stop","sleep"}));
    commands.clear(); assert(sensor.start());
    assert((commands==std::vector<std::string>{"wake","start"}));
    SensorData first;
    assert(!sensor.update(first) && std::isnan(first.pm2_5));
    sampleReady=true; assert(sensor.update(first) && first.pm2_5==4.5f);
    commands.clear(); assert(sensor.suspend());
    assert((commands==std::vector<std::string>{"stop","sleep"}));
    commands.clear(); assert(sensor.suspend() && commands.empty());
    assert(sensor.start());
    // Uma leitura perdida num novo ciclo não reutiliza a coleta anterior.
    sampleReady=false; SensorData next;
    assert(!sensor.update(next) && std::isnan(next.pm2_5));
    sampleReady=true; readError=9;
    assert(!sensor.update(next) && std::isnan(next.pm2_5));
    // Sem suporte a sleep, o sensor permanece Idle; não depende de cortar VCC.
    readError=0; sleepError=3; commands.clear(); assert(!sensor.suspend());
    assert((commands==std::vector<std::string>{"stop","sleep"}));
    commands.clear(); assert(sensor.start());
    assert((commands==std::vector<std::string>{"start"}));
    // Falha no stop não autoriza sleep; é registrada e pode ser repetida.
    stopError=7; commands.clear(); assert(!sensor.suspend());
    assert((commands==std::vector<std::string>{"stop"}));
    stopError=0; sleepError=0; assert(sensor.suspend());
    wakeError=2; commands.clear(); assert(!sensor.start());
    assert((commands==std::vector<std::string>{"wake"}));
    SensorData asleep; assert(!sensor.update(asleep) && std::isnan(asleep.pm2_5));
    std::cout << "SPS30 sleep/wake ordering, unsupported sleep, fresh data and failure checks passed\n";
}
