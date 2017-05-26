/*
 Name:		simpleserver.ino
 Created:	5/22/2017 11:31:15 PM
 Author:	Gert
*/

#include <Wire.h>
#include <Adafruit_ADS1015.h>
#include <ESP8266WiFi.h>
#include <SPIFFSEditor.h>
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);

// == I2C Analog board ADS1115
// https://www.adafruit.com/product/1085

static const int PIN_SCL = D5; // SCL; // 5
static const int PIN_SDA = D4; // SDA; // 4
Adafruit_ADS1115 analog;

// == Smooth

const float SMOOTH = 50;
const float SMOOTH_INIT = -999.9;

float smooth(float current, float newValue) {
	if (current == SMOOTH_INIT) {
		return newValue;
	}

	return ((current * (SMOOTH - 1.0)) + newValue) / SMOOTH;
}

// == Light ========================

float _light = SMOOTH_INIT;

float light() {
	return _light;
}

float measureLight() {
	analog.setGain(GAIN_TWO);
	long raw = analog.readADC_SingleEnded(0);
	double volt = (double)raw * 2.048 / (double)0x7FFF;
	return 100.0 * (2.048 - volt) / 2.048;
}

float updateLight() {
	return _light = smooth(_light, measureLight());
}

// == Temperature ==============

float _temp = SMOOTH_INIT;

float temperature() {
	return _temp;
}

float measureTemp() {
	// This will work for temperatures below 50°C
	analog.setGain(GAIN_FOUR);
	long raw = analog.readADC_SingleEnded(1);
	if (raw > 0x4000 && raw < 0x7FFF) {
		double volt = (double)raw * 1.024 / (double)0x7FFF;
		return (volt - 0.5) * 100;
	}
	if (raw == 0x7FFF) {
		// Temperature is above 50°C, and above 1V. This is
		// above the GAIN_FOUR range
		// We will loose 1 bit precision
		analog.setGain(GAIN_TWO);
		raw = analog.readADC_SingleEnded(1);
		double volt = (double)raw * 2.048 / (double)0x7FFF;
		return (volt - 0.5) * 100;
	}
	// We are below 0.5 V (below 0°C temperatures), we can double the precision here
	analog.setGain(GAIN_EIGHT);
	raw = analog.readADC_SingleEnded(1);
	double volt = (double)raw * 0.512 / (double)0x7FFF;
	return (volt - 0.5) * 100;
}

float updateTemparature() {
	return _temp = smooth(_temp, measureTemp());
}

// == LEDS ====

const int LED_R = D1; // Red
const int LED_G = D2; // Green
const int LED_B = D6; // Blue

const char *red() {
	return digitalRead(LED_R) ? "On" : "Off";
}

const char *green() {
	return digitalRead(LED_G) ? "On" : "Off";
}

const char *blue() {
	return digitalRead(LED_B) ? "On" : "Off";
}

// == Web Server ======================

void handlePost(AsyncWebServerRequest *request) {
	if (request->hasArg("toggle")) {
		String toggle = request->arg("toggle");
		if (toggle == "red") {
			digitalWrite(LED_R, !digitalRead(LED_R));
		}
		if (toggle == "green") {
			digitalWrite(LED_G, !digitalRead(LED_G));
		}
		if (toggle == "blue") {
			digitalWrite(LED_B, !digitalRead(LED_B));
		}
	}
}



// the setup function runs once when you press reset or power the board
void setup() {
	Serial.begin(115200);
	
	// Initialize electronics
	SPIFFS.begin();
	Wire.begin(PIN_SDA, PIN_SCL);
	analog.begin();

	pinMode(LED_R, OUTPUT);
	pinMode(LED_G, OUTPUT);
	pinMode(LED_B, OUTPUT);

	// Initialize WiFi
	WiFi.mode(WIFI_STA);
	WiFi.begin(); // Use cached SSID and password

	// Wait for connection
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
	Serial.println();
	Serial.println(WiFi.localIP());

	razor_setup(&server);
	server.on("/", index_cshtml);
	server.serveStatic("/site.css", SPIFFS, "/site.css");
	server.begin();
}

unsigned long nextMeasure = 0;
const unsigned long MEASURE_INTERVAL = 1000;

void loop() {
	if (millis() > nextMeasure) {
		while (millis() > nextMeasure) nextMeasure += MEASURE_INTERVAL;

		Serial.print("L: ");
		Serial.print(updateLight());
		Serial.println(" %");

		Serial.print("T: ");
		Serial.print(updateTemparature());
		Serial.println(" C");
	}
}
