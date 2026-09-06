#pragma once

#include <Arduino.h>
#include <string>


class PendingMeasurementStore
{
public:
    bool begin();

    bool enqueue(
        const std::string& payload
    );

    bool peek(
        std::string& payload
    );

    bool removeFirst();

    size_t count();

    bool isReady() const;


private:
    bool ready = false;

    size_t maxQueueBytes = 0;

    void recoverTemporaryFile();
};