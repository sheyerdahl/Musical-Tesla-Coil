#ifndef FREQUENCYSWEEP_H
#define FREQUENCYSWEEP_H

#include <Arduino.h>

namespace FrequencySweep {

    // Initialize the frequency sweep system
    void begin(uint8_t gd1aPin, uint8_t gd1bPin);
    
    // Start frequency sweep if triggered
    void handle();
    
    // Check if sweep is currently running
    bool isRunning();


    // Pin assignments
    extern uint8_t gd1aPin;
    extern uint8_t gd1bPin;
    
    // State tracking
    extern bool initialized;
    extern bool running;
    extern uint16_t currentFrequency;
    extern uint16_t minFrequency;
    extern uint16_t maxFrequency;
    extern uint32_t toggleCount;
    //extern unsigned long lastToggleTime;
    extern uint32_t toggleInterval;
    
    // Helper functions
    void togglePins();
    void startSweep(uint16_t minFreq, uint16_t maxFreq);
    void stopSweep();
};

#endif // FREQUENCYSWEEP_H
