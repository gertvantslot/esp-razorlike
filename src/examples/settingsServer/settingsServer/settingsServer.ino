/*
 Name:		settingsServer.ino
 Created:	6/6/2017 11:04:54 PM
 Author:	Gert
*/

// the setup function runs once when you press reset or power the board
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <ESPAsyncWebServer.h>
#include <EEPROM.h>

AsyncWebServer server(80);

// CalculateCRC32 from https://github.com/esp8266/Arduino/blob/master/libraries/esp8266/examples/RTCUserMemory/RTCUserMemory.ino 
uint32_t calculateCRC32(const uint8_t *data, size_t length)
{
	uint32_t crc = 0xffffffff;
	while (length--) {
		uint8_t c = *data++;
		for (uint32_t i = 0x80; i > 0; i >>= 1) {
			bool bit = crc & 0x80000000;
			if (c & i) {
				bit = !bit;
			}
			crc <<= 1;
			if (bit) {
				crc ^= 0x04c11db7;
			}
		}
	}
	return crc;
}

#define SETTINGS_ADDR 0
struct Settings {
	bool led_red;
	bool led_green;
	bool led_blue;
	int light_min;
	int light_max;
	char page_title[50];
	char email[100];
	uint32 crc32;
};

Settings settings;

void initSettings() {
	settings.led_red = settings.led_green = settings.led_blue = false;
	settings.light_min = 20;
	settings.light_max = 50;
	strcpy(settings.page_title, "<<Settings demo>>");
	strcpy(settings.email, "");
}

void updateSettingsCRC(struct Settings * s) {
	int crc = calculateCRC32((const uint8_t *)s, sizeof(Settings) - sizeof(uint32));
	s->crc32 = crc;
}

void updateSettingsCRC() {
	updateSettingsCRC(&settings);
}

bool getSettingsFromEEPROM() {
	struct Settings buffer;
	EEPROM.get(SETTINGS_ADDR, buffer);
	uint32 storedCRC = buffer.crc32;
	Serial.print("Received CRC:");
	Serial.println(buffer.crc32);

	updateSettingsCRC(&buffer);
	Serial.print("calculated CRC:");
	Serial.println(buffer.crc32);
	uint32 calculatedCRC = buffer.crc32;
	if (storedCRC == calculatedCRC) {
		Serial.println("Valid settings found in EEPROM");
		// EEPROM contains a valid settings object
		settings = buffer;
		return true;
	} 
	Serial.println("INVALID settings found in EEPROM - no update");
	return false;
}

void putSettingsInEEPROM() {
	Serial.println("Settings will be stored");
	Serial.print("Putting CRC:");
	Serial.println(settings.crc32);
	EEPROM.put(SETTINGS_ADDR, settings);
	EEPROM.commit();
	// Retrieve to see if correctly stored
	getSettingsFromEEPROM();
}

bool settingsNeedUpdate() {
	struct Settings buffer;
	EEPROM.get(SETTINGS_ADDR, buffer);
	return settings.crc32 != buffer.crc32;
}

void putSettingsInEEPROMifNeeded() {
	updateSettingsCRC(&settings);
	if (settingsNeedUpdate()) {
		Serial.println("EEPROM settings update needed");
		putSettingsInEEPROM();
	}
}

// == LEDS ====

const int LED_R = D1; // Red
const int LED_G = D2; // Green
const int LED_B = D6; // Blue

void setup() {
	Serial.begin(115200);

	// Initialize electronics
	EEPROM.begin(512);
	SPIFFS.begin();

	pinMode(LED_R, OUTPUT);
	pinMode(LED_G, OUTPUT);
	pinMode(LED_B, OUTPUT);

	// Retrieve settings if available and correct
	initSettings();
	getSettingsFromEEPROM();

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
	server.on("/red", HTTP_GET, [](AsyncWebServerRequest *request) {
		settings.led_red = !settings.led_red;
		request->send(200, "text/plain", String(settings.led_red));
	});
	server.on("/green", HTTP_GET, [](AsyncWebServerRequest *request) {
		settings.led_green = !settings.led_green;
		request->send(200, "text/plain", String(settings.led_green));
	});
	server.on("/blue", HTTP_GET, [](AsyncWebServerRequest *request) {
		settings.led_blue = !settings.led_blue;
		request->send(200, "text/plain", String(settings.led_blue));
	});
	server.begin();
}

void onIndexPost(AsyncWebServerRequest *request) {
	strlcpy(settings.page_title, request->arg("page_title").c_str(), 50);
	settings.led_red = strncmp("on", request->arg("red").c_str(), 2) == 0;
	settings.led_green = strncmp("on", request->arg("green").c_str(), 2) == 0;
	settings.led_blue = strncmp("on", request->arg("blue").c_str(), 2) == 0;
	settings.light_min = atoi(request->arg("light_min").c_str());
	settings.light_max = atoi(request->arg("light_max").c_str());
	strlcpy(settings.email, request->arg("email").c_str(), 100);
	
	if (request->arg("submit").length() != 0) {
		putSettingsInEEPROMifNeeded();
	}
}

void onIndexRequest(AsyncWebServerRequest *request) {
	if (request->method() == WebRequestMethod::HTTP_POST) {
		onIndexPost(request);
	}
}

// the loop function runs over and over again until power down or reset
void loop() {
	digitalWrite(LED_R, settings.led_red ? HIGH : LOW);
	digitalWrite(LED_G, settings.led_green);
	digitalWrite(LED_B, settings.led_blue);
}
