#ifndef VBUS_H
#define VBUS_H

#include <Arduino.h>

namespace VBus {
    // Initialize the current transformer with the specified pin
    void begin(uint8_t VBusPin, uint16_t externalResistanceKiloOhms);
    
    // Read the current transformer value (multisampled)
    uint16_t readVBus();

    extern uint8_t VBusPin;
    extern uint16_t externalResistanceKiloOhms;
};

#endif // CURRENTTRANSFORMER_H
