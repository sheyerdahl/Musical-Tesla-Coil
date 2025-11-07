#include "VBus.h"
#include "FastAnalogRead.h"

// Constants
const uint8_t multisampleCount = 10;
const uint8_t vBusOpAmpResistanceKiloOhms = 3;

namespace VBus {
    uint8_t VBusPin = 0;
    uint16_t externalResistanceKiloOhms = 0;

    void begin(uint8_t newVBusPin, uint16_t newExternalResistanceKiloOhms) {
        VBusPin = newVBusPin;
        externalResistanceKiloOhms = newExternalResistanceKiloOhms;
    }

    uint16_t readVBus() {
    if (VBusPin == 0) {
        return 0;
    }
    
    uint32_t sum = 0;
    
    // Take readings and average them
    for (int i = 0; i < multisampleCount; i++) {
        sum += analogReadMilliVoltsFast(VBusPin);
    }
    
    // Return the averaged result
    uint16_t average = sum / multisampleCount;
    uint16_t busVoltageMilivolts = average * (externalResistanceKiloOhms / vBusOpAmpResistanceKiloOhms);
    return busVoltageMilivolts / 1000;
}
}
