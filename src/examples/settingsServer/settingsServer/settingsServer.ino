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
bool reboot = false;
bool wifi_emergency = false;
unsigned long emergency_mode_end;

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
	bool wifi_sta;
	char ssid_name[100];
	char ssid_password[100];
	char emergency_ssid_name[100];
	char emergency_ssid_password[100];
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
#define SSID_NAME_DEFAULT "example"
#define SSID_PASSWORD_DEFAULT "example password ##"

void initSettings() {
	settings.wifi_sta = false;
	strcpy(settings.ssid_name, SSID_NAME_DEFAULT);
	strcpy(settings.ssid_password, SSID_PASSWORD_DEFAULT);
	strcpy(settings.emergency_ssid_name, SSID_NAME_DEFAULT);
	strcpy(settings.emergency_ssid_password, SSID_PASSWORD_DEFAULT);

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

	updateSettingsCRC(&buffer);
	uint32 calculatedCRC = buffer.crc32;
	if (storedCRC == calculatedCRC) {
		// EEPROM contains a valid settings object
		// settings = buffer;
		memcpy(&settings, &buffer, sizeof(Settings));
		return true;
	} 
	return false;
}

void putSettingsInEEPROM() {
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
		putSettingsInEEPROM();
	}
}

// == LEDS ====

const int LED_R = D1; // Red
const int LED_G = D2; // Green
const int LED_B = D6; // Blue

void setup() {
	Serial.begin(115200);
	Serial.println();
	Serial.println("Example - Settings");

	// Initialize electronics
	EEPROM.begin(sizeof(Settings));
	SPIFFS.begin();

	pinMode(LED_R, OUTPUT);
	pinMode(LED_G, OUTPUT);
	pinMode(LED_B, OUTPUT);

	// Retrieve settings if available and correct
	initSettings();
	if (getSettingsFromEEPROM()) {
		Serial.println("Usable settings found in EEPROM");
	}

	// Initialize WiFi
	if (settings.wifi_sta) {
		Serial.println("Connection to AP");
		WiFi.mode(WIFI_STA);
		WiFi.begin(settings.ssid_name, settings.ssid_password); // Use cached SSID and password
	
		// Wait for connection
		unsigned long maxWaitTime = millis() + 10000; // 10 seconds
		while (WiFi.status() != WL_CONNECTED) {
			delay(500);
			Serial.print(".");
			if (millis() > maxWaitTime) {
				// Still no connection
				Serial.println("no connection. Start emergency mode");
				// In emergency mode, you are still able to connect to the device
				// so you can control it, to connect it to another SSID.
				// For safety, we use the stored password.
				wifi_emergency = true;
				emergency_mode_end = 10 * 60 * 1000; // retry normal ap in 10 minutes
				break;
			}
		}
		Serial.println();
		Serial.println(WiFi.localIP());
	}
	
	if (wifi_emergency || !settings.wifi_sta) {
		Serial.println("Starting AP mode");
		char * ssid = wifi_emergency ? settings.emergency_ssid_name : settings.ssid_name;
		char * password = wifi_emergency ? settings.emergency_ssid_password : settings.ssid_password;

		WiFi.mode(WIFI_AP);
		WiFi.softAP(ssid, password);
		WiFi.begin();

		Serial.print("SSID: ");
		Serial.println(ssid);
		Serial.println(WiFi.softAPIP());
	}


	razor_setup(&server);
	server.on("/", index_cshtml);
	server.serveStatic("/js/", SPIFFS, "/js/").setCacheControl("max-age=600");
	server.serveStatic("/css/", SPIFFS, "/css/").setCacheControl("max-age=600");

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
	strlcpy(settings.email, request->arg("email").c_str(), 100);

	settings.led_red = strncmp("on", request->arg("red").c_str(), 2) == 0;
	settings.led_green = strncmp("on", request->arg("green").c_str(), 2) == 0;
	settings.led_blue = strncmp("on", request->arg("blue").c_str(), 2) == 0;

	settings.light_min = atoi(request->arg("light_min").c_str());
	settings.light_max = atoi(request->arg("light_max").c_str());

	// Wifi Settings

	bool sta = strncmp("sta", request->arg("wifi_mode").c_str(), 3) == 0;
	if (sta != settings.wifi_sta) {
		settings.wifi_sta = sta;
		reboot = true;
	}
	String name = request->arg("ssid_name");
	if (strncmp(settings.ssid_name, name.c_str(), 100) != 0) {
		strlcpy(settings.ssid_name, name.c_str(), 100);
		reboot = true;
	}
	String password = request->arg("ssid_password");
	if (password.length() != 0) {
		strlcpy(settings.ssid_password, password.c_str(), 100);
		reboot = true;
	}

	strlcpy(settings.emergency_ssid_name, request->arg("emergency_ssid_name").c_str(), 100);
	String epassword = request->arg("emergency_ssid_password");
	if (epassword.length() != 0) {
		strlcpy(settings.emergency_ssid_password, epassword.c_str(), 100);
	}

	if (request->arg("submit").length() != 0) {
		putSettingsInEEPROMifNeeded();
	}

	if (request->arg("reboot").length() != 0) {
		reboot = true;
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

	if (wifi_emergency && millis() > emergency_mode_end) {
		reboot = true;
	}

	if (reboot) {
		reboot = false;
		Serial.println("bye");

		// This will fail the first time after flashing.
		// First do a manual reboot (button)
		// See: https://github.com/esp8266/Arduino/issues/2876

		ESP.reset();
		Serial.println("I'm now a zombie");
	}
}
