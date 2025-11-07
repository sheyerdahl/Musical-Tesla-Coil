// Inclues
#include <Arduino.h>
//#include "WifiOta.h"
#include "BleControl.h"
#include "MidiControl.h"
#include "ZCD.h"
#include "FastAnalogRead.h"
#include "CurrentTransformer.h"
#include "GateDrive.h"
#include "FrequencySweep.h"
#include "Relay.h"
#include "Burst.h"
#include "VBus.h"
#include "OCD.h"

// Constants
const uint16_t startFrequency = 400; // In KHz
const uint16_t maxBurstLength = 200; // In uS
const uint16_t externalResistanceKiloOhms = 400;
const uint16_t ocdCurrent = 400;
const uint16_t ctTurnsRatio = 512;
const uint32_t ctBurdenMiliohms = 3300;

// Pins
// const uint8_t CurrentTransformerPin = 2;
// const uint8_t ZCDAPin = 3;
// const uint8_t ZCDBPin = 4;
const uint8_t Therm1Pin = 5;
const uint8_t Therm2Pin = 6;
const uint8_t PrimaryRelayPin = 11;
const uint8_t BypassRelayPin = 12;
const uint8_t GD1APin = 13;
const uint8_t GD1BPin = 14;
const uint8_t GD2APin = 15;
const uint8_t GD2BPin = 16;
const uint8_t ZCDInterruptPin = 21;
const uint8_t CTPeakResetPin = 45;

// Analog Pins
const uint8_t CTPeakPin = 9;
const uint8_t VbusPin = 1;

// Variables
float test = 0.0f;
unsigned long lastMicros = 0;

uint32_t adcSamples = 0;

TaskHandle_t Task0;

// Functions
void setup() {
	Serial.begin(115200);
	// while (!Serial) {
	// 	delay(100);
	// }
	Serial.println("Starting up...");

	// Initialize the FastADC library
    fadcInit(2, CTPeakPin, VbusPin);
	//WifiOta::begin(WIFI_SSID, WIFI_PASSWORD, "tesla-coil");

	GateDrive::begin(GD1APin, GD1BPin, GD2APin, GD2BPin);
	BleControl::begin("TeslaCoil");
	MidiControl::begin();
	ZCD::begin(ZCDInterruptPin, GD1APin, GD1BPin);
	CurrentTransformer::begin(CTPeakPin, CTPeakResetPin);
	FrequencySweep::begin(GD1APin, GD1BPin);
	Relay::begin(PrimaryRelayPin, BypassRelayPin);
	VBus::begin(VbusPin, externalResistanceKiloOhms);
	OCD::begin(ocdCurrent, ctTurnsRatio, ctBurdenMiliohms);
}

void loop() {
  	//WifiOta::handle();
	FrequencySweep::handle();
	Burst::handle();

	BleControl::ControlState controlState = BleControl::getState();
	(void)controlState;
	// Read sensors (placeholder analogReads; adjust scaling as needed)
	// float vbus = analogRead(VbusPin);
	//float ct = CurrentTransformer::readCurrentTransformer();
	// float t1 = analogRead(Therm1Pin);
	// float t2 = analogRead(Therm2Pin);
	//float cpuFrequency = getCpuFrequencyMhz();
	uint16_t vBusVoltage = VBus::readVBus();
	BleControl::notifyReadings(vBusVoltage, test, 0, 0);
	Relay::setEnabled(controlState.enabled);
	// Serial.print("Enabled: ");
	// Serial.print(s.enabled);
	// Serial.print(", Burst Length: ");
	// Serial.print(s.burstLength);
	// Serial.print(", BPS: ");
	// Serial.print(s.bps);
	// Serial.println();

	test += float(micros() - lastMicros) * 0.000001f;
	lastMicros = micros();
	if (test > 4.0f) {
		// Serial.print("Test reset, core is: ");
		// Serial.println(xPortGetCoreID());
		test = 0.0f;
		size_t freeHeap = ESP.getFreeHeap();
		//size_t largestFreeBlock = ESP.getMaxAllocHeap();
		size_t freePsram = ESP.getFreePsram();
		//size_t largestFreeBlockPsram = ESP.getMaxAllocPsram();
		Serial.print("Free heap: ");
		Serial.print(freeHeap);
		Serial.print(", Free PSRAM: ");
		Serial.print(freePsram);
		Serial.println(" bytes");
	}
	delay(100);
	//delayMicroseconds(1);
}