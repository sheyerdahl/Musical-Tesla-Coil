#ifndef Relay_H
#define Relay_H

#include <Arduino.h>

namespace Relay {
    void begin(uint8_t primaryRelayPin, uint8_t bypassRelayPin);
    void setEnabled(bool enabled);

    extern uint8_t _primaryRelayPin;
    extern uint8_t _bypassRelayPin;
    extern bool _enabled;
}

#endif
