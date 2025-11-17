#include "MidiControl.h"
#include "BleControl.h"
#include <sstream>
#include "MidiFile.h"

// Constants
//Note frequency lookup table
const uint32_t Midi_NoteFreq_dHz[] = {82, 87, 92, 97, 103, 109, 116, 122, 130, 138, 146, 154, 164, 173, 184, 194, 206, 218, 231, 245, 260, 275, 291, 309, 327, 346, 367, 389, 412, 437, 462, 490, 519, 550, 583, 617, 654, 693, 734, 778, 824, 873, 925, 980, 1038, 1100, 1165, 1235, 1308, 1386, 1468, 1556, 1648, 1746, 1850, 1960, 2077, 2200, 2331, 2469, 2616, 2772, 2937, 3111, 3296, 3492, 3700, 3920, 4153, 4400, 4662, 4939, 5233, 5544, 5873, 6223, 6593, 6985, 7400, 7840, 8306, 8800, 9323, 9878, 10465, 11087, 11747, 12445, 13185, 13969, 14800, 15680, 16612, 17600, 18647, 19755, 20930, 22175, 23493, 24890, 26370, 27938, 29600, 31360, 33224, 35200, 37293, 39511, 41860, 44349, 46986, 49780, 52740, 55877, 59199, 62719, 66449, 70400, 74586, 79021, 83720, 88698, 93973, 99561, 105481, 111753, 118398, 125439};

namespace MidiControl {
    std::vector<uint8_t> midiBuffer;
	bool fileReady = false;
	bool transferInProgress = false;
	
	// Playback state
	bool isPlaying = false;
	smf::MidiFile midiFile;
	bool midiFileLoaded = false;
	unsigned long playbackStartTime = 0;
	unsigned long chordSwitchTimestampMs = 0;
	size_t currentEventIndex = 0;
    TaskHandle_t playMidiTaskHandle;
	//std::vector<uint8_t> onNotes = {};
	uint8_t onNotes[5] = {0, 0, 0, 0, 0};
	uint8_t currentPlayingNote = 0;

	void begin() {
		midiBuffer.clear();
		fileReady = false;
		transferInProgress = false;
		playMidiTaskHandle = NULL;
	}
	
	bool receiveChunk(const uint8_t* data, size_t length) {
		if (data == nullptr || length == 0) {
			// Empty chunk signals end of transfer
			if (transferInProgress) {
				fileReady = true;
				transferInProgress = false;
				// Reset loaded state so new file will be loaded on next play
				// midiFileLoaded = false;
				// sortedEvents.clear();
				// isPlaying = false;
				return true;
			}
			return false;
		}
		
		// If starting a new transfer, clear old data
		if (!transferInProgress) {
			midiBuffer.clear();
			midiFileLoaded = false;
			//sortedEvents.clear();
			//isPlaying = false;
            setPlaying(false);
		}
		
		// Append chunk to buffer
		transferInProgress = true;
		midiBuffer.insert(midiBuffer.end(), data, data + length);
		
		// Transfer is still in progress, not complete yet
		return false;
	}
	
	bool isFileReady() {
		return fileReady;
	}
	
	const std::vector<uint8_t>& getMidiData() {
		return midiBuffer;
	}
	
	std::istringstream createInputStream() {
		// Create a string from the vector data
		std::string midiString(reinterpret_cast<const char*>(midiBuffer.data()), midiBuffer.size());
		std::istringstream stream(midiString);
		return stream;
	}
	
	void clear() {
		midiBuffer.clear();
		fileReady = false;
		transferInProgress = false;
		// Reset playback state
		//isPlaying = false;
        setPlaying(false);
		midiFileLoaded = false;
		//sortedEvents.clear();
		//midiFile.clear();
		currentEventIndex = 0;
	}
	
	size_t getBufferSize() {
		return midiBuffer.size();
	}
	
	void setPlaying(bool playing) {
		if (playing && !isPlaying) {
			// Start playback
			Serial.println("setPlaying true");
			if (!midiFileLoaded && isFileReady()) {
				Serial.println("Loading MidiFile");
				// Load MIDI file
				// Between HERE
				std::istringstream stream = createInputStream();
				if (midiFile.read(stream)) {
					Serial.println("MidiFile loaded");
					midiFileLoaded = true;
					midiFile.doTimeAnalysis();
					midiFile.linkNotePairs();
					//midiFile.joinTracks();
					
					currentEventIndex = 0;
					playbackStartTime = millis();
				}
			}
			
			if (midiFileLoaded) {
				isPlaying = true;
				currentEventIndex = 0;
				playbackStartTime = millis();
				// Make sure we don't have a dangling task handle
				if (playMidiTaskHandle != NULL) {
					vTaskDelete(playMidiTaskHandle);
					playMidiTaskHandle = NULL;
				}
				// and HERE is where the heap is being gobbled
				// Check available heap before creating task
				size_t freeHeap = ESP.getFreeHeap();
				//size_t largestFreeBlock = ESP.getMaxAllocHeap();
				size_t freePsram = ESP.getFreePsram();
				Serial.print("Free heap before task creation: ");
				Serial.print(freeHeap);
				Serial.print(", Free PSRAM: ");
				Serial.print(freePsram);
				Serial.println(" bytes");
				
				BaseType_t result = xTaskCreate(playMidiTask, "playMidiTask", 4096, NULL, 3, &playMidiTaskHandle);
				if (result == pdPASS) {
					Serial.print("playMidiTask created successfully with ");
				} else {
					Serial.print("Failed with stack size ");
					Serial.println(result);
				}
				
				Serial.print("Free heap AFTER task creation: ");
				Serial.print(freeHeap);
				Serial.print(", Free PSRAM: ");
				Serial.print(freePsram);
				Serial.println(" bytes");
			}
		} else if (!playing && isPlaying) {
			Serial.println("deleting playMidiTask...");
			// Stop playback
			isPlaying = false;
			if (playMidiTaskHandle != NULL) {
				vTaskDelete(playMidiTaskHandle);
				playMidiTaskHandle = NULL;
			}
			clearOnNotes();
			Serial.println("playMidiTask deleted");
		}
	}
	
	void addOnNote(uint8_t note) {
		for (uint8_t i = 0; i < 5; i++) {
			uint8_t onNote = onNotes[i];
			if (onNote == 0) {
				onNotes[i] = note;
				break;
			}
		}
	}

	void removeOnNote(uint8_t note) {
		for (uint8_t i = 0; i < 5; i++) {
			uint8_t onNote = onNotes[i];
			if (onNote == note) {
				onNotes[i] = 0;
				break;
			}
		}

		if (note == currentPlayingNote) {
			stopNote();
		}
	}

	void clearOnNotes() {
		stopNote();
		for (uint8_t i = 0; i < 5; i++) {
			onNotes[i] = 0;
		}
	}

	void playNote(uint8_t note) {
		BleControl::ControlState controlState = BleControl::getState();
		int8_t octave = controlState.midiOctave;
		uint32_t noteFreq = Midi_NoteFreq_dHz[note] / 10;

		if (octave > 0) {
			noteFreq *= 1 << octave;
		} else if (octave < 0) {
			noteFreq /= 1 << -octave;
		}

		BleControl::setBps(noteFreq);
		currentPlayingNote = note;
	}

	void stopNote() {
		currentPlayingNote = 0;
		BleControl::setBps(1);
	}

	int8_t getIndexOfOnNote(uint8_t note) {
		for (uint8_t i = 0; i < 5; i++) {
			uint8_t onNote = onNotes[i];
			if (onNote == note) {
				return i;
			}
		}

		return -1;
	}

	void playNextOnNote() {
		if (currentPlayingNote == 0) {
			return;
		}

		int8_t noteIndex = getIndexOfOnNote(currentPlayingNote);
		if (noteIndex == -1) {
			return;
		}

		bool noteFound = false;
		// Find first note after the current one
		for (uint8_t i = noteIndex + 1; i < 5; i++) {
			uint8_t onNote = onNotes[i];
			if (onNote != 0) {
				noteFound = true;
				playNote(onNote);
				break;
			}
		}

		// Loop back around if the previous loop didn't find a note.
		if (!noteFound) {
			for (uint8_t i = 0; i < noteIndex; i++) {
				uint8_t onNote = onNotes[i];
				if (onNote != 0) {
					playNote(onNote);
					break;
				}
			}
		}
	}

	uint8_t getNumOnNotes() {
		uint8_t numOnNotes = 0;
		for (uint8_t i = 0; i < 5; i++) {
			uint8_t onNote = onNotes[i];
			if (onNote != 0) {
				numOnNotes++;
			}
		}
		return numOnNotes;
	}

	void handle() {
		if (!isPlaying || !midiFileLoaded) {
			return;
		}
		
		unsigned long currentTime = millis();
		float currentSeconds = (currentTime - playbackStartTime) / 1000.0;
		
		// Process all events that should have occurred by now
		smf::MidiEvent* event = NULL;
		while (currentEventIndex + 1 < midiFile[0].size()) {
			event = &midiFile[0][currentEventIndex];
			
			// Check if this event's time has arrived
			if (event->seconds <= currentSeconds) {
				// Extract value from MIDI event
				// For note-on events, use the note number (getP1() or getKeyNumber())
				if (event->isNoteOn() && event->getVelocity() > 0) {
					int noteValue = event->getKeyNumber();
					if (noteValue >= 0 && noteValue <= 127) {
						addOnNote(noteValue);
					}
				} else if ((event->isNoteOff() || (event->isNoteOn() && event->getVelocity() == 0))) {
					// Note off - could set bps to 1 or keep last value
					// For now, we'll just process note-on events
                    //BleControl::setBps(1);
					removeOnNote(event->getKeyNumber());
				}
				
				currentEventIndex++;
			} else {
				// Future event, wait
				break;
			}
		}
		
		// Check if we've reached the end
		if (currentEventIndex >= midiFile[0].size()) {
			//isPlaying = false;
            setPlaying(false);
			return;
		}

		// Handle On notes
		BleControl::ControlState controlState = BleControl::getState();
		for (uint8_t i = 0; i < 5; i++) {
			uint8_t onNote = onNotes[i];
			if (onNote == 0 || onNote == currentPlayingNote) {
				continue;
			}
			
			if (currentPlayingNote == 0) {
				playNote(onNote);
			} else if (millis() - chordSwitchTimestampMs >= controlState.chordSwapTime) {
				chordSwitchTimestampMs = millis();
				playNextOnNote();
			}
		}
		
		// Stop playing when there's no notes
		if (getNumOnNotes() == 0 && currentPlayingNote != 0) {
			stopNote();
		} 
	}

    void playMidiTask(void * arg) {
        while(isPlaying){
            handle();
            delay(1);
        }
    }
}

