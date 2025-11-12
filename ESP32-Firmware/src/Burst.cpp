#include "Arduino.h"
#include "Burst.h"
#include "ZCD.h"
#include "BleControl.h"
#include "GateDrive.h"
#include "FrequencySweep.h"
#include "DelayNanoseconds.h"
#include "OCD.h"

namespace Burst {
    // Constants
    
    // Variables
    bool burstEnabled = 0;
    TaskHandle_t burstTaskHandle;
    unsigned long lastBurstMillis;

    void handle() {
        BleControl::ControlState controlState = BleControl::getState();
        if (burstEnabled == controlState.burstEnabled || FrequencySweep::running) {
            return;
        }

        if (controlState.burstEnabled) {
            enable();
        } else {
            disable();
        }
    }

    void burstTaskLoop(void * arg) {
        while(burstEnabled){
            BleControl::ControlState controlState = BleControl::getState();
            uint32_t burstFrequencyHz = 1000000 / controlState.burstLength;
            uint16_t maxBurstsPerSecond = burstFrequencyHz / 15; // Max 7.5% duty cycle
            uint16_t burstsPerSecond = constrain(controlState.bps, 1, maxBurstsPerSecond);
            if ((millis() - lastBurstMillis) * 10 >= (10000 / burstsPerSecond)) {
                lastBurstMillis = millis();
                
                if (!OCD::ocdTriggered || controlState.burstLength <= 100) {
                    singleBurst();
                } else {
                    OCD::resetOCDTriggered();
                }
            }
            
            delay(1);
        }
    }

    void IRAM_ATTR singleBurst() {
        BleControl::ControlState controlState = BleControl::getState();
        uint16_t burstLength = constrain(controlState.burstLength, 10, 500); // In microseconds
        //uint32_t burstLengthCycles = burstLength * cpuFrequencyMHz;
        //unsigned long startMicros = micros();

        // ZCD::enableInterrupt();
        GateDrive::enableGD1();
        delayMicroseconds(4);
        //delayNanoseconds(8333);
        GateDrive::toggleGD1();
        ZCD::enable(false);

        //delayNanoseconds(burstLength * 1000);
        //while (micros() - startMicros < burstLength) {}
        delayMicroseconds(burstLength);

        ZCD::disableOnInterrupt();
        //GateDrive::disableGD1();
        OCD::checkOCD();
    }

    void enable() {
        xTaskCreate(burstTaskLoop, "burstTaskLoop", 2000, NULL, 2, &burstTaskHandle);
        burstEnabled = 1;
        ZCD::enableInterrupt();
    }

    void disable() {
        vTaskDelete(burstTaskHandle);
        burstEnabled = 0;
        ZCD::disableInterrupt();
    }
}