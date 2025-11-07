#ifndef ZCD_H
#define ZCD_H

#include <Arduino.h>

class ZCD {
public:
    // Initialize the ZCD system
    static void begin(uint8_t interruptPin, uint8_t gd1aPin, uint8_t gd1bPin);
    
    // Enable/disable the interrupt
    static IRAM_ATTR void enable(bool enableGD1 = true);
    static IRAM_ATTR void disable(bool disableGD1 = true);
    static IRAM_ATTR void disableOnInterrupt();
    static IRAM_ATTR void enableInterrupt();
    static IRAM_ATTR void disableInterrupt();
    
    // Check if interrupt is enabled
    static bool isEnabled();

private:
    // Interrupt service routine
    static void IRAM_ATTR interruptHandler();
    
    // Pin assignments
    static uint8_t _interruptPin;
    static uint8_t _gd1aPin;
    static uint8_t _gd1bPin;
    
    // State tracking
    static bool _enabled;
    static volatile bool _interruptOccurred;
    static bool _disableOnInterrupt;
};

#endif // ZCD_H
