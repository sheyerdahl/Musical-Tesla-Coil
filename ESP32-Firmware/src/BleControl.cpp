#include "BleControl.h"
#include "MidiControl.h"

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

namespace {
	BLE2902* pid2902 = new BLE2902();
	// UUIDs (randomly generated).
	const char* SERVICE_UUID =  "08160660-e062-460c-8834-06f539975761"; // insert uuid here
	const char* UUID_VBUS =     "18160660-e062-460c-8834-06f539975761"; // float notify
	const char* UUID_CT =       "28160660-e062-460c-8834-06f539975761"; // float notify
	const char* UUID_THERM1 =   "38160660-e062-460c-8834-06f539975761"; // float notify
	const char* UUID_THERM2 =   "48160660-e062-460c-8834-06f539975761"; // float notify
	const char* UUID_TOGGLE =   "58160660-e062-460c-8834-06f539975761"; // bool write
	const char* UUID_BURST = "68160660-e062-460c-8834-06f539975761"; // u16 write
	const char* UUID_BPS =      "78160660-e062-460c-8834-06f539975761"; // u16 write
	const char* UUID_BURST_ENABLED = "88160660-e062-460c-8834-06f539975761"; // bool write
	const char* UUID_PHASE_LEAD = "98160660-e062-460c-8834-06f539975761"; // u16 write
	const char* UUID_REVERSE_BURST_PHASE = "a8160660-e062-460c-8834-06f539975761"; // bool write
	const char *MIDI_UPLOAD = "b8160660-e062-460c-8834-06f539975761"; // chunked write
	const char *PLAY_MIDI = "c8160660-e062-460c-8834-06f539975761"; // bool write
	const char *UUID_MIDI_OCTAVE = "d8160660-e062-460c-8834-06f539975761"; // int8 write

	const char* FREQUENCY_SWEEP_SERVICE_UUID =  "08160661-e062-460c-8834-06f539975761"; // insert uuid here
	const char* UUID_MIN_FREQ_SWEEP = "08160662-e062-460c-8834-06f539975761"; // u16 write
	const char* UUID_MAX_FREQ_SWEEP = "08160663-e062-460c-8834-06f539975761"; // u16 write
	const char* UUID_START_FREQ_SWEEP = "08160664-e062-460c-8834-06f539975761"; // bool write
	const char* UUID_FREQ_SWEEP_DATA = "08160665-e062-460c-8834-06f539975761"; // u32 read

	const BLEUUID* serviceBLEUUID = new BLEUUID(SERVICE_UUID);

	BLEServer* server = nullptr;
	BLEService* service = nullptr;
	BLEService* frequencySweepService = nullptr;
	BLECharacteristic* chVbus = nullptr;
	BLECharacteristic* chCt = nullptr;
	BLECharacteristic* chTherm1 = nullptr;
	BLECharacteristic* chTherm2 = nullptr;
	BLECharacteristic* chToggle = nullptr;
	BLECharacteristic* chBurst = nullptr;
	BLECharacteristic* chBps = nullptr;
	BLECharacteristic* chBurstEnabled = nullptr;
	BLECharacteristic* chPhaseLead = nullptr;
	BLECharacteristic* chReverseBurstPhase = nullptr;
	BLECharacteristic* chMinFreqSweep = nullptr;
	BLECharacteristic* chMaxFreqSweep = nullptr;
	BLECharacteristic* chStartFreqSweep = nullptr;
	BLECharacteristic* chFreqSweepData = nullptr;
	BLECharacteristic* chMidiUpload = nullptr;
	BLECharacteristic* chPlayMidi = nullptr;
	BLECharacteristic* chMidiOctave = nullptr;

	BleControl::ControlState state { false, 100, 2, 90, 110, false, false, 0, false, 0 };

	class ControlCallbacks : public BLECharacteristicCallbacks {
		void onWrite(BLECharacteristic* characteristic) override {
			std::string value = characteristic->getValue();
			// Serial.println("Characteristic was written to:");
			// Serial.println(characteristic == chStartFreqSweep);
			// Serial.println(value.size());
			if (characteristic == chToggle) {
				state.enabled = (!value.empty() && value[0] != 0);
			} else if (characteristic == chBurst) {
				if (value.size() >= 2) {
					uint16_t v = ((uint8_t)value[0]) | (((uint8_t)value[1]) << 8);
					state.burstLength = v;
				}
			} else if (characteristic == chBps) {
				if (value.size() >= 2) {
					uint16_t v = ((uint8_t)value[0]) | (((uint8_t)value[1]) << 8);
					state.bps = v;
				}
			} else if (characteristic == chMinFreqSweep) {
				if (value.size() >= 2) {
					uint16_t v = ((uint8_t)value[0]) | (((uint8_t)value[1]) << 8);
					state.minFrequencySweep = v;
				}
			} else if (characteristic == chMaxFreqSweep) {
				if (value.size() >= 2) {
					uint16_t v = ((uint8_t)value[0]) | (((uint8_t)value[1]) << 8);
					state.maxFrequencySweep = v;
				}
			} else if (characteristic == chStartFreqSweep) {
				state.startFrequencySweep = (!value.empty() && (uint8_t)value[0] != 0);
			} else if (characteristic == chBurstEnabled) {
				state.burstEnabled = (!value.empty() && (uint8_t)value[0] != 0);
			} else if (characteristic == chPhaseLead) {
				if (value.size() >= 2) {
					uint16_t v = ((uint8_t)value[0]) | (((uint8_t)value[1]) << 8);
					state.phaseLead = v;
				}
			} else if (characteristic == chReverseBurstPhase) {
				state.reverseBurstPhase = (!value.empty() && (uint8_t)value[0] != 0);
			} else if (characteristic == chMidiUpload) {
				// Forward chunk to MidiControl
				Serial.println("MidiUpload");
				Serial.print(value.size());
				Serial.print(" | ");
				Serial.println(value.data());
				if (!value.empty()) {
					MidiControl::receiveChunk((const uint8_t*)value.data(), value.size());
				} else {
					// Empty chunk signals end of transfer
					MidiControl::receiveChunk(nullptr, 0);
				}
			} else if (characteristic == chPlayMidi) {
				bool playMidi = (!value.empty() && (uint8_t)value[0] != 0);
				MidiControl::setPlaying(playMidi);
			} else if (characteristic == chMidiOctave) {
				if (!value.empty()) {
					int8_t octave = (int8_t)value[0];
					state.midiOctave = octave;
				}
			}
		}
	};
}

namespace BleControl {
	void begin(const char* deviceName) {
		BLEDevice::init(deviceName);
		server = BLEDevice::createServer();
		
		service = server->createService(*serviceBLEUUID, 40); // Characteristics take 2 handles, descriptors take 1 handle. Default is 15 handles.
		frequencySweepService = server->createService(FREQUENCY_SWEEP_SERVICE_UUID);

		// Service characteristics
		chCt = service->createCharacteristic(
			UUID_CT,
			BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ
		);
		chVbus = service->createCharacteristic(
			UUID_VBUS,
			BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ
		);
		chTherm1 = service->createCharacteristic(
			UUID_THERM1,
			BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ
		);
		chTherm2 = service->createCharacteristic(
			UUID_THERM2,
			BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ
		);
		chToggle = service->createCharacteristic(
			UUID_TOGGLE,
			BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ
		);
		chBurst = service->createCharacteristic(
			UUID_BURST,
			BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ
		);
		chBps = service->createCharacteristic(
			UUID_BPS,
			BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ
		);
		chBurstEnabled = service->createCharacteristic(
			UUID_BURST_ENABLED,
			BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ
		);
		chPhaseLead = service->createCharacteristic(
			UUID_PHASE_LEAD,
			BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ
		);
		chReverseBurstPhase = service->createCharacteristic(
			UUID_REVERSE_BURST_PHASE,
			BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ
		);
		chMidiUpload = service->createCharacteristic(
			MIDI_UPLOAD,
			BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ
		);
		chPlayMidi = service->createCharacteristic(
			PLAY_MIDI,
			BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ
		);
		chMidiOctave = service->createCharacteristic(
			UUID_MIDI_OCTAVE,
			BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ
		);
		
		// frequencySweepService characteristics
		chMinFreqSweep = frequencySweepService->createCharacteristic(
			UUID_MIN_FREQ_SWEEP,
			BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ
		);
		chMaxFreqSweep = frequencySweepService->createCharacteristic(
			UUID_MAX_FREQ_SWEEP,
			BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ
		);
		chStartFreqSweep = frequencySweepService->createCharacteristic(
			UUID_START_FREQ_SWEEP,
			BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ
		);
		chFreqSweepData = frequencySweepService->createCharacteristic(
			UUID_FREQ_SWEEP_DATA,
			BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ
		);
		
		static ControlCallbacks cb;
		chToggle->setCallbacks(&cb);
		chBurst->setCallbacks(&cb);
		chBps->setCallbacks(&cb);
		chBurstEnabled->setCallbacks(&cb);
		chPhaseLead->setCallbacks(&cb);
		chReverseBurstPhase->setCallbacks(&cb);
		chMinFreqSweep->setCallbacks(&cb);
		chMaxFreqSweep->setCallbacks(&cb);
		chStartFreqSweep->setCallbacks(&cb);
		chMidiUpload->setCallbacks(&cb);
		chPlayMidi->setCallbacks(&cb);
		chMidiOctave->setCallbacks(&cb);

		chFreqSweepData->addDescriptor(pid2902);
		// chVbus->addDescriptor(pid2902);
		// chCt->addDescriptor(pid2902);
		// chTherm1->addDescriptor(pid2902);
		// chTherm2->addDescriptor(pid2902);
		
		service->start();
		frequencySweepService->start();
		BLEAdvertising* advertising = BLEDevice::getAdvertising();
		advertising->addServiceUUID(*serviceBLEUUID);
		advertising->addServiceUUID(FREQUENCY_SWEEP_SERVICE_UUID);
		advertising->setScanResponse(true);
		advertising->setMinPreferred(0x06);
		advertising->setMaxPreferred(0x12);
		BLEDevice::startAdvertising();
	}

	void handle() {
		// No periodic handling needed for GATT server beyond notifications, handled elsewhere
	}

	static void setFloat(BLECharacteristic* ch, float v, bool notify) {
		ch->setValue((uint8_t*)&v, sizeof(float));
		if (notify) ch->notify();
	}

	void notifyReadings(float vbus, float currentTransformer, float therm1, float therm2) {
		if (chVbus) setFloat(chVbus, vbus, true);
		if (chCt) setFloat(chCt, currentTransformer, true);
		if (chTherm1) setFloat(chTherm1, therm1, true);
		if (chTherm2) setFloat(chTherm2, therm2, true);
	}

	void IRAM_ATTR notifyFrequencySweepData(uint32_t data) {
		if (chFreqSweepData) {
			chFreqSweepData->setValue((uint8_t*)&data, sizeof(uint32_t));
			chFreqSweepData->notify();
		}
	}

	void resetStartFrequencySweep() {
		state.startFrequencySweep = false;
	}

	void setBurstEnabled(bool newBurstEnabled) {
		state.burstEnabled = newBurstEnabled;
	}

	void setBps(uint16_t newBps) {
		state.bps = newBps;
	}

	ControlState IRAM_ATTR getState() {
		return state;
	}
}


