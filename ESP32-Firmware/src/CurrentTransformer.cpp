#include "CurrentTransformer.h"
#include "FastAnalogRead.h"

// Constants
const uint8_t numAnalogReadings = 5;
const uint8_t resetDuration = 5; // In micro seconds

// Static member variable definitions
uint8_t CurrentTransformer::_CTPeakPin = 0;
uint8_t CurrentTransformer::_CTPeakResetPin = 0;
bool CurrentTransformer::_initialized = false;

void CurrentTransformer::begin(uint8_t CTPeakPin, uint8_t CTPeakResetPin) {
    _CTPeakPin = CTPeakPin;
    _CTPeakResetPin = CTPeakResetPin;
    
    pinMode(_CTPeakResetPin, OUTPUT);
    digitalWrite(_CTPeakResetPin, 0);

    _initialized = true;
}

uint16_t IRAM_ATTR CurrentTransformer::readCurrentTransformer() {
    if (!_initialized) {
        return 0;
    }
    
    uint32_t sum = 0;
    
    // Take readings and average them
    for (int i = 0; i < numAnalogReadings; i++) {
        sum += analogReadMilliVoltsFast(_CTPeakPin);
        digitalWrite(_CTPeakResetPin, 1);
        delayMicroseconds(resetDuration);
        digitalWrite(_CTPeakResetPin, 0);
    }
    
    // Return the averaged result
    return (uint16_t)(sum / numAnalogReadings);
}
