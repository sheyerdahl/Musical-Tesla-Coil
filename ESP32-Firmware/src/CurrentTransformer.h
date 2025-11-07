#ifndef CURRENTTRANSFORMER_H
#define CURRENTTRANSFORMER_H

#include <Arduino.h>

class CurrentTransformer {
public:
    // Initialize the current transformer with the specified pin
    static void begin(uint8_t CTPeakPin, uint8_t CTPeakResetPin);
    
    // Read the current transformer value (averaged over 5 samples)
    static uint16_t readCurrentTransformer();

private:
    static uint8_t _CTPeakPin;
    static uint8_t _CTPeakResetPin;
    static bool _initialized;
};

#endif // CURRENTTRANSFORMER_H
