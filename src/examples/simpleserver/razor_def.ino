#include <ESPAsyncWebServer.h>

#define _RAZOR_BUF_SIZE 512
bool _razor_fromFile(AsyncResponseStream* response, File* file, int start, int end) {
	uint8_t buf[_RAZOR_BUF_SIZE];
	int len = end - start;

	//Serial.print("Sending file from: ");
	//Serial.print(start);
	//Serial.print(" - ");
	//Serial.print(len);

	int todo = len;
	int p = file->position();
	file->seek(start, SeekMode::SeekSet);

	while (todo > 0) {
		int avail = file->read((uint8_t*)buf, todo > _RAZOR_BUF_SIZE ? _RAZOR_BUF_SIZE : todo);
		response->write(buf, avail);
		todo -= avail;
	}
	//Serial.println(" - OK");
	return true;
}

bool _razor_fromExpression(AsyncResponseStream* response, const float expression) {
	response->print(expression);
}

bool _razor_fromExpression(AsyncResponseStream* response, const double expression) {
	response->print(expression);
}

bool _razor_fromExpression(AsyncResponseStream* response, const int expression) {
	response->print(expression);
}

bool _razor_fromExpression(AsyncResponseStream* response, const long expression) {
	response->print(expression);
}

bool _razor_fromExpression(AsyncResponseStream* response, const char * expression) {
	response->print(expression);
}

bool _razor_fromExpression(AsyncResponseStream* response, const Printable& expression) {
	response->print(expression);
}
