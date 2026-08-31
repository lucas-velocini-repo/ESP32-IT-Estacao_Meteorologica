#include "device-identity.h"

String DeviceIdentity::getHardwareId()
{
    uint64_t chipId = ESP.getEfuseMac();

    char buffer[17];

    snprintf(
        buffer,
        sizeof(buffer),
        "%04X%08X",
        static_cast<uint16_t>(chipId >> 32),
        static_cast<uint32_t>(chipId)
    );

    return String(buffer);
}