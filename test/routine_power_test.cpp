#include <cassert>
#include <cmath>
#include <iostream>
#include "routines/routine-controller.h"
#include "led/status-leds.h"
#include "power/power-manager.h"

SerialStub Serial;
uint64_t fakeNow=0;
std::map<uint8_t,int> pwm;
RuntimeCalls calls;
bool sourceHigh=false;
uint32_t requestedSleep=0;
bool sleepingWithLed=false;

namespace PowerManager {
void begin() {}
bool externalPower() { return sourceHigh; }
void internalLedsOff() {}
void gnssPower(bool) {}
void batteryCpu() {}
void normalCpu() {}
uint64_t nowMs() { return fakeNow; }
void idleBattery(uint32_t ms,bool led) { requestedSleep=ms; sleepingWithLed=led; }
}
// Spy no limite da rotina: o controlador real decide se pode iniciar os rádios.
void NormalRoutine::begin() { ++calls.normalBegins; }
void NormalRoutine::update() { ++calls.normalUpdates; }
void NormalRoutine::end() { ++calls.normalEnds; }

void reset() { calls=RuntimeCalls(); fakeNow=0; sourceHigh=false; pwm.clear(); }
void onlyPowerRed(bool on) {
    for (const auto& entry:pwm)
        assert((entry.second>0)==(on && entry.first==Pins::LED3_RED));
}

int main() {
    // Boot frio na bateria não passa por normal, GNSS ou teste.
    reset();
    RoutineController battery(false);
    battery.begin();
    battery.update();
    assert(calls.initialBattery && calls.normalBegins==0 && calls.contextBegins==1);
    assert(calls.starts==1 && calls.saved.empty());
    onlyPowerRed(true);
    fakeNow=1000; battery.update(); onlyPowerRed(false);
    assert(requestedSleep==4000 && !sleepingWithLed);
    fakeNow=29999; battery.update(); assert(calls.saved.empty());
    fakeNow=30000; battery.update(); assert(calls.lightStarts==1 && calls.saved.empty());
    fakeNow=30180; battery.update(); assert(calls.saved.size()==1);
    assert(calls.saved.back().pm2_5==12 && calls.suspends==1);
    fakeNow=299999; battery.update(); assert(calls.starts==1);
    fakeNow=300000; battery.update(); assert(calls.starts==2);

    // Retorno da fonte durante estabilização cancela a coleta incompleta.
    sourceHigh=true; fakeNow=300100; battery.update();
    fakeNow=300149; battery.update(); assert(calls.normalBegins==0);
    fakeNow=300150; battery.update();
    assert(calls.normalBegins==1 && calls.suspends==2 && calls.saved.size()==1);
    sourceHigh=false; fakeNow=300200; battery.update();
    fakeNow=300250; battery.update();
    assert(calls.normalEnds==1 && calls.contextBegins==1 && calls.starts==3);

    // Fonte oscilando por menos de 50 ms não reinicializa a rotina.
    sourceHigh=true; fakeNow+=10; battery.update();
    sourceHigh=false; fakeNow+=10; battery.update();
    assert(calls.normalBegins==1 && calls.normalEnds==1);

    // SPS30 sem dados não salva zeros nem bloqueia indefinidamente os outros sensores.
    reset(); calls.pmAvailable=false;
    RoutineController missing(false); missing.begin(); missing.update();
    fakeNow=30000; missing.update();
    fakeNow=30180; missing.update(); assert(calls.saved.empty());
    fakeNow=35180; missing.update(); assert(calls.saved.size()==1);
    assert(std::isnan(calls.saved.back().pm2_5) && calls.saved.back().temperature==25);

    // Intervalo curto mantém a estabilização e um tempo útil de repouso.
    reset(); calls.intervalSeconds=30;
    RoutineController shortInterval(false); shortInterval.begin(); shortInterval.update();
    fakeNow=30000; shortInterval.update(); fakeNow=30180; shortInterval.update();
    fakeNow=60179; shortInterval.update(); assert(calls.starts==1);
    fakeNow=60180; shortInterval.update(); assert(calls.starts==2);

    // Modo de teste tem prioridade sobre a fonte e não abre armazenamento/radio.
    reset(); RoutineController test(true); test.begin(); test.update();
    sourceHigh=true; test.update();
    assert(calls.testBegins==1 && calls.testUpdates==2);
    assert(calls.contextBegins==0 && calls.normalBegins==0 && calls.starts==0);

    // HIGH no boot entra diretamente no normal; relógio de 64 bits na bateria.
    reset(); sourceHigh=true; RoutineController normal(false); normal.begin(); normal.update();
    assert(!calls.initialBattery && calls.normalBegins==1 && calls.normalUpdates==1);
    sourceHigh=false; fakeNow=(1ULL<<32)+100; normal.update();
    fakeNow+=50; normal.update(); assert(calls.normalEnds==1 && calls.starts==1);
    fakeNow+=30000; normal.update(); fakeNow+=180; normal.update();
    assert(calls.saved.size()==1);

    // Mesmo controle PWM nos dois tipos de LED; somente a fonte pisca na bateria.
    StatusLeds::begin(); StatusLeds::normal(true,true);
    const int blue=pwm[ Pins::LED3_BLUE ];
    assert(blue>0 && blue<255);
    StatusLeds::battery(999); onlyPowerRed(true);
    assert(pwm[Pins::LED3_RED]>0 && pwm[Pins::LED3_RED]<255 && pwm[Pins::LED3_RED]!=blue);
    StatusLeds::battery(1000); onlyPowerRed(false);
    StatusLeds::battery(4999); onlyPowerRed(false);
    StatusLeds::battery(5000); onlyPowerRed(true);
    std::cout << "Routine selection, source transitions, battery sampling, timeout and PWM checks passed\n";
}
