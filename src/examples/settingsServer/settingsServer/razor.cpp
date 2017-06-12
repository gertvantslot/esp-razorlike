// 
// 
// 

#include "razor.h"

HTMLString Raw(const char * expression) {
	return HTMLString(expression);
}

#define _RAZOR_BUF_SIZE 512
bool _razor_fromFile(AsyncResponseStream* response, File* file, int start, int end) {
	uint8_t buf[_RAZOR_BUF_SIZE];
	int len = end - start;

	int todo = len;
	int p = file->position();
	file->seek(start, SeekMode::SeekSet);

	while (todo > 0) {
		int avail = file->read((uint8_t*)buf, todo > _RAZOR_BUF_SIZE ? _RAZOR_BUF_SIZE : todo);
		response->write(buf, avail);
		todo -= avail;
	}
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
	const char * current = expression;
	while (*current) {
		char c = *current;
		switch (c)
		{
		case '<': response->print("&lt;"); break;
		case '>': response->print("&gt;"); break;
		case '&': response->print("&amp;"); break;
		default:
			response->print(c);
			break;
		}
		current++;
	}
}

bool _razor_fromExpression(AsyncResponseStream* response, const HTMLString expression) {
	response->print(expression);
}

bool _razor_fromExpression(AsyncResponseStream* response, const Printable& expression) {
	response->print(expression);
}
