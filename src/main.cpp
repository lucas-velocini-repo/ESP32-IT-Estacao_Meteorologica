#include <Arduino.h>
#include "routines/routine-controller.h"

// true: hardware-test.h. false: seleciona rotina normal/bateria por POWER_SOURCE.
constexpr bool TEST_MODE = false;
RoutineController routines(TEST_MODE);

void setup()
{
    routines.begin();
}

void loop()
{
    routines.update();
}
