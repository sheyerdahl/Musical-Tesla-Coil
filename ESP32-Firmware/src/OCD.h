#ifndef OCD_H
#define OCD_h

#include <Arduino.h>

namespace OCD {
    void begin(uint16_t newOCDCurrent, uint16_t newTurnsRatio, uint32_t newBurdenMiliohms);
    void checkOCD();
    void resetOCDTriggered();

    extern uint16_t OCDCurrent;
    extern uint16_t turnsRatio;
    extern uint32_t burdenMiliohms;
    extern bool ocdTriggered;
}

#endif