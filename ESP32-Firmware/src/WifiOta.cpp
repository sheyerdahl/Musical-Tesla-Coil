// #include <Arduino.h>
// #include "WifiOta.h"
// #include <WiFi.h>
// #include <ArduinoOTA.h>

// namespace {
// 	static uint8_t s_progressLedPin = 255;
// }

// namespace WifiOta {
// 	void begin(const char* ssid, const char* password, const char* hostname) {

// 		WiFi.mode(WIFI_STA);
// 		WiFi.begin(ssid, password);

// 		unsigned long start = millis();
// 		while (WiFi.status() != WL_CONNECTED) {
// 			delay(250);
// 			if (millis() - start > 20000) {
// 				break;
// 			}
// 		}

// 		//ArduinoOTA.setHostname(hostname);
// 		ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
// 			(void)total;
// 		});
		
// 		if (WiFi.status() == WL_CONNECTED) {
// 			ArduinoOTA.begin();
// 			Serial.print("OTA Ready. IP: ");
// 			Serial.println(WiFi.localIP());
// 			Serial.println(ArduinoOTA.getHostname());
// 		} else {
// 			Serial.println("WiFi not connected. OTA disabled.");
// 		}
// 	}

// 	void handle() {
// 		ArduinoOTA.handle();
// 	}
// }


