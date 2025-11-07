#ifndef BURST_H
#define BURST_H

#include <Arduino.h>

namespace Burst {
    void handle();
    void IRAM_ATTR singleBurst();
    void enable();
    void disable();
    void burstTaskLoop();

    extern bool burstEnabled;
    extern TaskHandle_t burstTaskHandle;
    extern unsigned long lastBurstMillis;
}

#endif
