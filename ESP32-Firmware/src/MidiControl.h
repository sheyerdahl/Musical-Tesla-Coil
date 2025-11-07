#pragma once

#include <Arduino.h>
#include <vector>
#include <string>
#include <sstream>
#include <istream>

namespace MidiControl {
	// Initialize MIDI control
	void begin();
	
	// Handle receiving a chunk of MIDI data (up to 20 bytes)
	// Returns true if the transfer is complete and file is ready
	bool receiveChunk(const uint8_t* data, size_t length);
	
	// Check if a MIDI file is ready to be read
	bool isFileReady();
	
	// Get the MIDI file data as a vector (for direct access)
	// Caller must check isFileReady() first
	const std::vector<uint8_t>& getMidiData();
	
	// Create an istringstream from the MIDI data for use with MidiFile::read()
	// Caller must check isFileReady() first
	// Note: This creates a copy of the data, use getMidiData() if you need zero-copy access
	std::istringstream createInputStream();
	
	// Clear the current MIDI file buffer
	void clear();
	
	// Get the size of the current MIDI file buffer
	size_t getBufferSize();
	
	// Set playback state (start/stop)
	void setPlaying(bool playing);
	
	// Handle MIDI playback (call from loop())
	void handle();
    void playMidiTask(void * arg);

    extern TaskHandle_t playMidiTaskHandle;
}

