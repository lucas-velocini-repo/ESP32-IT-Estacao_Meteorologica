#pragma once

#include <Arduino.h>
#include <string>


class PendingMeasurementStore
{
public:
    PendingMeasurementStore(const char* queue = "/pending.jsonl", const char* temp = "/pending.tmp")
        : queueFilePath(queue), tempFilePath(temp) {}
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
    const char* queueFilePath;
    const char* tempFilePath;
    bool ready = false;

    size_t maxQueueBytes = 0;

    void recoverTemporaryFile();
};