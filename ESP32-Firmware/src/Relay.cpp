#include "Arduino.h"
#include "Relay.h"

namespace Relay {
    // Variable definitions
    uint8_t _primaryRelayPin = 0;
    uint8_t _bypassRelayPin = 0;
    bool _enabled = false;
    void begin(uint8_t primaryRelayPin, uint8_t bypassRelayPin) {
        _primaryRelayPin = primaryRelayPin;
        _bypassRelayPin = bypassRelayPin;
        _enabled = false;

        pinMode(_primaryRelayPin, OUTPUT);
        pinMode(_bypassRelayPin, OUTPUT);
        digitalWrite(_primaryRelayPin, 0);
        digitalWrite(_bypassRelayPin, 0);
    }

    void setEnabled(bool newEnabled) {
        if (_enabled == newEnabled) {
            return;
        }

        digitalWrite(_primaryRelayPin, newEnabled);
        if (newEnabled) {
            delay(2000);
        }
        digitalWrite(_bypassRelayPin, newEnabled);
        _enabled = newEnabled;
    }
}