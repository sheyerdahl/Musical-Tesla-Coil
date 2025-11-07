#include "OCD.h"
#include "CurrentTransformer.h"
#include "BleControl.h"


namespace OCD {
    uint16_t OCDCurrent = 0;
    uint16_t turnsRatio = 0;
    uint32_t burdenMiliohms = 0;

    void begin(uint16_t newOCDCurrent, uint16_t newTurnsRatio, uint32_t newBurdenMiliohms) {
        OCDCurrent = newOCDCurrent;
        turnsRatio = newTurnsRatio;
        burdenMiliohms = newBurdenMiliohms;
    }

    void handleOCD() {
        uint32_t ctMilivolts = CurrentTransformer::readCurrentTransformer();
        uint16_t ctCurrent = (ctMilivolts * turnsRatio) / burdenMiliohms; // I = (V * turns ratio)/R, because V = (I / turns ratio)R
        if (ctCurrent >= OCDCurrent) {
            //Burst::disable();
            BleControl::setBurstEnabled(0);
            delay(200);
            BleControl::setBurstEnabled(1);
        }
    }
}
