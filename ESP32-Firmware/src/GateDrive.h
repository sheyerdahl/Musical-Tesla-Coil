#ifndef GateDrive_H
#define GateDrive_H

#include <Arduino.h>

namespace GateDrive {
    void begin(uint8_t GD1APin, uint8_t GD1BPin, uint8_t GD2APin, uint8_t GD2BPin);
    void toggleGD1();
    void enableGD1();
    void disableGD1();

    extern bool GD1APinEnabled;
    extern bool GD1BPinEnabled;
    extern uint8_t GD1APin;
    extern uint8_t GD1BPin;
}

#endif
