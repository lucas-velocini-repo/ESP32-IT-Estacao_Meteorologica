#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "Wire.h"
constexpr int SPS30_I2C_ADDR_69=0x69;
constexpr int SPS30_OUTPUT_FORMAT_OUTPUT_FORMAT_FLOAT=0;
extern std::vector<std::string> commands;
extern bool sampleReady;
extern int stopError,sleepError,wakeError,startError,readError;
struct SensirionI2cSps30 {
    void begin(WireStub&,int) { commands.push_back("bind"); }
    int16_t wakeUpSequence() { commands.push_back("wake"); return wakeError; }
    int16_t stopMeasurement() { commands.push_back("stop"); return stopError; }
    int16_t startMeasurement(int) { commands.push_back("start"); return startError; }
    int16_t sleep() { commands.push_back("sleep"); return sleepError; }
    int16_t readDataReadyFlag(uint16_t& ready) { ready=sampleReady?1:0; return 0; }
    int16_t readMeasurementValuesFloat(float& a,float& b,float& c,float& d,
        float& e,float& f,float& g,float& h,float& i,float& j) {
        if (readError) return readError;
        a=b=c=d=e=f=g=h=i=j=4.5f; return 0;
    }
};
