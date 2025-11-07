#pragma once

#include <Arduino.h>

namespace WifiOta {
	void begin(const char* ssid, const char* password, const char* hostname);
	void handle();
}


