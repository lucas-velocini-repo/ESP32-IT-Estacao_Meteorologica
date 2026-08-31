#pragma once

#include <Arduino.h>

class DeviceIdentity
{
public:
    static String getHardwareId();
};