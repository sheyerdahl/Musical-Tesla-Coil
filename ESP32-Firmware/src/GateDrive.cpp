#include "GateDrive.h"
#include "BleControl.h"

namespace GateDrive {
    bool GD1APinEnabled = 0;
    bool GD1BPinEnabled = 0;
    uint8_t GD1APin = 0;
    uint8_t GD1BPin = 0;

    void begin(uint8_t newGD1APin, uint8_t newGD1BPin, uint8_t newGD2APin, uint8_t newGD2BPin) {
        GD1APin = newGD1APin;
        GD1BPin = newGD1BPin;

        // Configure pins
        pinMode(GD1APin, OUTPUT);
        pinMode(GD1BPin, OUTPUT);
        
        // Set initial states (low, low)
        digitalWrite(GD1APin, 0);
        digitalWrite(GD1BPin, 0);
    }

    void IRAM_ATTR toggleGD1() {
        GD1APinEnabled = !GD1APinEnabled;
        GD1BPinEnabled = !GD1BPinEnabled;
        digitalWrite(GD1APin, GD1APinEnabled);
        digitalWrite(GD1BPin, GD1BPinEnabled);
    }

    void IRAM_ATTR enableGD1() {
        BleControl::ControlState state = BleControl::getState();
        GD1APinEnabled = state.reverseBurstPhase ? 1 : 0;
        GD1BPinEnabled = state.reverseBurstPhase ? 0 : 1;
        digitalWrite(GD1APin, GD1APinEnabled);
        digitalWrite(GD1BPin, GD1BPinEnabled);
    }

    void IRAM_ATTR disableGD1() {
        GD1APinEnabled = 0;
        GD1BPinEnabled = 0;
        digitalWrite(GD1APin, GD1APinEnabled);
        digitalWrite(GD1BPin, GD1BPinEnabled);
    }
}