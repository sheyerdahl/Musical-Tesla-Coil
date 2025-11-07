#include "FrequencySweep.h"
#include "BleControl.h"
#include "CurrentTransformer.h"
#include "GateDrive.h"
#include <hal/cpu_hal.h>

namespace FrequencySweep {
    // Constants
    const uint32_t cpuFrequencyHz = getCpuFrequencyMhz() * 1000000;
    const uint32_t magnitude32Bit = 4294967295;
    const unsigned long frequencySweepDurationMs = 20000;
    // Variable definitions
    uint8_t gd1aPin = 0;
    uint8_t gd1bPin = 0;
    bool initialized = false;
    bool running = false;
    uint16_t currentFrequency = 0;
    uint16_t minFrequency = 0;
    uint16_t maxFrequency = 0;
    uint32_t toggleCount = 0;
    //unsigned long lastToggleTime = 0;
    uint32_t lastCycleCount = ESP.getCycleCount();
    uint32_t cyclesNeededToToggle = 0;
    TaskHandle_t Task0;

    void begin(uint8_t newgd1aPin, uint8_t newgd1bPin) {
        gd1aPin = newgd1aPin;
        gd1bPin = newgd1bPin;
        
        initialized = true;
    }
    // TODO: there's a smidge of extra on time at the start of a burst, and the output is left high at the end, causing extra pulses / oscillations
    void handle() {
        if (!initialized) {
            return;
        }
        
        // Check BLE state for frequency sweep trigger
        BleControl::ControlState state = BleControl::getState();
        
        if (!state.startFrequencySweep || running || state.burstEnabled) {
            return;
        }

        BleControl::resetStartFrequencySweep();
        startSweep(state.minFrequencySweep, state.maxFrequencySweep);
    }

    bool isRunning() {
        return running;
    }

    uint32_t IRAM_ATTR getCyclesNeededToToggle(uint32_t toggleFrequencyKHz) {
        return cpuFrequencyHz / (toggleFrequencyKHz * 2000);
    }

    void IRAM_ATTR Task0SweepLoop() {
        //(void)parameter; // Suppress unused parameter warning
        // lastCycleCount = ESP.getCycleCount();
        lastCycleCount = cpu_hal_get_cycle_count();
        while (running) {
            // uint32_t currentCycleCount = ESP.getCycleCount();
            uint32_t currentCycleCount = cpu_hal_get_cycle_count();
            
            // Check if it's time to toggle
            uint32_t compare = currentCycleCount >= lastCycleCount ? currentCycleCount - lastCycleCount : ((magnitude32Bit - currentCycleCount) + 1) - lastCycleCount;
            if (compare >= cyclesNeededToToggle) {
                // If we've completed 20 toggles at this frequency, move to next frequency
                if (toggleCount >= 20) {
                    uint16_t ctValue = CurrentTransformer::readCurrentTransformer();
                    uint32_t sweepData = ((uint32_t)currentFrequency << 16) + ctValue; // Frequency then ctValue are merged into a single uint32
                    BleControl::notifyFrequencySweepData(sweepData);

                    currentFrequency++;
                    toggleCount = 0;
                    // If we've reached the max frequency, stop the sweep
                    if (currentFrequency > maxFrequency) {
                        stopSweep();
                        GateDrive::disableGD1();
                        break;
                    }
                    // Calculate new toggle interval for the next frequency
                    //cyclesNeededToToggle = (uint32_t)1000000 / ((uint32_t)currentFrequency * 2000); // Convert KHz to microseconds
                    cyclesNeededToToggle = getCyclesNeededToToggle(currentFrequency);

                    delay(100);
                }

                // digitalWrite(gd1aPin, !digitalRead(gd1aPin));
                // digitalWrite(gd1bPin, !digitalRead(gd1bPin));
                GateDrive::toggleGD1();
                // lastCycleCount = ESP.getCycleCount();
                lastCycleCount = cpu_hal_get_cycle_count();
                toggleCount++;
            }
        }
    }

    void startSweep(uint16_t minFreq, uint16_t maxFreq) {
        Serial.println("Starting sweep!");

        minFreq = constrain(minFreq, 20, 999);
        maxFreq = constrain(maxFreq, 30, 999);
        
        Serial.println(minFreq);
        Serial.println(maxFreq);
        minFrequency = minFreq;
        maxFrequency = maxFreq;
        currentFrequency = minFreq;
        toggleCount = 0;
        //lastCycleCount = ESP.getCycleCount();
        // lastToggleTime = micros();
        
        // Calculate toggle interval for the starting frequency (KHz to microseconds)
        //cyclesNeededToToggle = (uint32_t)1000000 / ((uint32_t)currentFrequency * 2000);
        cyclesNeededToToggle = getCyclesNeededToToggle(currentFrequency);
        Serial.println(cyclesNeededToToggle);
        running = true;

        //unsigned long millisStarted = millis();
        //xTaskCreatePinnedToCore(Task0SweepLoop, "Task0", 10000, NULL, 1, &Task0, 0);
        //void *a = NULL;
        GateDrive::enableGD1();
        Task0SweepLoop();
    }

    void stopSweep() {
        running = false;
        
        // Set pins to safe state (both low)
        // digitalWrite(gd1aPin, LOW);
        // digitalWrite(gd1bPin, LOW);
        
        // Notify BLE that sweep is complete
        //BleControl::notifyFrequencySweepData(currentFrequency);
    }
}
