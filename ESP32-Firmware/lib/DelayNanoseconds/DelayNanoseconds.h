#ifndef DELAYNANOSECONDS_H
#define DELAYNANOSECONDS_H

#include <Arduino.h>

#define delayNanoseconds(nanoseconds) \
    const uint32_t cpuFrequencyMHz = getCpuFrequencyMhz(); \
    uint32_t nanosecondsCycles = (nanoseconds * cpuFrequencyMHz) / 1000; \
    uint32_t startCycleCount = ESP.getCycleCount(); \
    while (true) { \
        uint32_t currentCycleCount = ESP.getCycleCount(); \
        uint32_t compare = currentCycleCount >= startCycleCount ? currentCycleCount - startCycleCount : (((uint32_t)4294967295 - currentCycleCount) + 1) - startCycleCount; \
        if (compare >= nanosecondsCycles) { \
            break; \
        } \
    }

#endif