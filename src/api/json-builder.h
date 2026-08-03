#pragma once

#include <string>

#include "station/station-data.h"

class JsonBuilder
{
public:
    static std::string buildStationPayload(
        const StationData& station
    );
};