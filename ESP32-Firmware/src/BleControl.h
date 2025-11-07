#pragma once

#include <Arduino.h>

namespace BleControl {
	struct ControlState {
		bool enabled;
		uint16_t burstLength;
		uint16_t bps;
		uint16_t minFrequencySweep;
		uint16_t maxFrequencySweep;
		bool startFrequencySweep;
		bool burstEnabled;
		uint16_t phaseLead; // In nanoseconds
		bool reverseBurstPhase;
		int8_t midiOctave;
	};

	void begin(const char* deviceName);
	void handle();
	void notifyReadings(float vbus, float currentTransformer, float therm1, float therm2);
	void notifyFrequencySweepData(uint32_t data);
	void resetStartFrequencySweep();
	void setBurstEnabled(bool newBurstEnabled);
	void setBps(uint16_t newBps);
	ControlState getState();
}


