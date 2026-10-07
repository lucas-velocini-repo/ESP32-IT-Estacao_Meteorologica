#pragma once
#include <cstdint>
#include <string>
using String = std::string;
// Adapt Arduino String's emptiness method to std::string in this host harness.
#define isEmpty empty
struct SerialStub { void println(const char*) {} };
extern SerialStub Serial;
