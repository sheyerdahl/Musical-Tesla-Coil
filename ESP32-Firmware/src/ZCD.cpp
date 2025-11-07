#include "ZCD.h"
#include "GateDrive.h"
#include "BleControl.h"
#include "DelayNanoseconds.h"

// Constants
const uint32_t cpuFrequencyMHz = getCpuFrequencyMhz();
const uint32_t magnitude32Bit = 4294967295;

// Static member variable definitions
uint8_t ZCD::_interruptPin = 0;
uint8_t ZCD::_gd1aPin = 0;
uint8_t ZCD::_gd1bPin = 0;
bool ZCD::_enabled = false;
bool ZCD::_disableOnInterrupt = false;
//volatile bool ZCD::_interruptOccurred = false;

void ZCD::begin(uint8_t interruptPin, uint8_t gd1aPin, uint8_t gd1bPin) {
    _interruptPin = interruptPin;
    _gd1aPin = gd1aPin;
    _gd1bPin = gd1bPin;
    
    // Configure interrupt pin
    pinMode(_interruptPin, INPUT);
    
    // Attach interrupt for both rising and falling edges
    //attachInterrupt(digitalPinToInterrupt(_interruptPin), interruptHandler, CHANGE);
    
    
    // _enabled = true;
}

void IRAM_ATTR ZCD::enableInterrupt() {
    attachInterrupt(digitalPinToInterrupt(_interruptPin), interruptHandler, CHANGE);
}

void IRAM_ATTR ZCD::disableInterrupt() {
    detachInterrupt(digitalPinToInterrupt(_interruptPin));
}

void IRAM_ATTR ZCD::enable(bool enableGD1) {
    if (_interruptPin != 0) {
        _enabled = true;
        if (enableGD1) {
            GateDrive::enableGD1();
        }
    }
}

void IRAM_ATTR ZCD::disable(bool disableGD1) {
    if (_interruptPin != 0) {
        _enabled = false;
        if (disableGD1) {
            GateDrive::disableGD1();
        }
        //disableInterrupt();
    }
}

void IRAM_ATTR ZCD::disableOnInterrupt() {
    _disableOnInterrupt = true;
}

bool IRAM_ATTR ZCD::isEnabled() {
    return _enabled;
}

void IRAM_ATTR ZCD::interruptHandler() {
    if (!_enabled) {
        return;
    }
    // Phase lead
    BleControl::ControlState controlState = BleControl::getState();
    uint16_t phaseLead = constrain(controlState.phaseLead, 0, 1250);
    // uint32_t phaseLeadCycles = (controlState.phaseLead * cpuFrequencyMHz) / 1000; // Phase lead is in nanoseconds, so convert to cycles
    // uint32_t startCycleCount = ESP.getCycleCount();

    delayNanoseconds(phaseLead);
    // while (true) {
    //     uint32_t currentCycleCount = ESP.getCycleCount();
    //     uint32_t compare = currentCycleCount >= startCycleCount ? currentCycleCount - startCycleCount : ((magnitude32Bit - currentCycleCount) + 1) - startCycleCount;
    //     if (compare >= phaseLeadCycles) {
    //         break;
    //     }
    // }

    if (_disableOnInterrupt) {
        _disableOnInterrupt = false;
        disable();
        return;
    }

    // Toggle both GD1A and GD1B pins
    // digitalWrite(_gd1aPin, !digitalRead(_gd1aPin));
    // digitalWrite(_gd1bPin, !digitalRead(_gd1bPin));
    GateDrive::toggleGD1();

    //_interruptOccurred = true;
}
