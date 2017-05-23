/*
 Name:		simpleserver.ino
 Created:	5/22/2017 11:31:15 PM
 Author:	Gert
*/

#include <WebResponseImpl.h>
#include <WebHandlerImpl.h>
#include <WebAuthentication.h>
#include <StringArray.h>
#include <SPIFFSEditor.h>
#include <ESPAsyncWebServer.h>
#include <AsyncWebSocket.h>
#include <AsyncJson.h>
#include <AsyncEventSource.h>

float light() {
	// Measure light
	return 0.3;
}

float temperature() {
	// Measure temperature
	return 20.0;
}

void handlePost(AsyncWebServerRequest *request) {

}

// the setup function runs once when you press reset or power the board
void setup() {

}

// the loop function runs over and over again until power down or reset
void loop() {
  
}
