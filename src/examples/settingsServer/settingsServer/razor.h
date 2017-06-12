// razor.h

#ifndef _RAZOR_h
#define _RAZOR_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include "WString.h"
#include "ESPAsyncWebServer.h"

class HTMLString : public String
{
public:
	HTMLString(const char *cstr = "") : String(cstr) {
	}
};

HTMLString Raw(const char * expression);

#define _RAZOR_BUF_SIZE 512
bool _razor_fromFile(AsyncResponseStream* response, File* file, int start, int end);
bool _razor_fromExpression(AsyncResponseStream* response, const float expression);
bool _razor_fromExpression(AsyncResponseStream* response, const double expression);
bool _razor_fromExpression(AsyncResponseStream* response, const int expression);
bool _razor_fromExpression(AsyncResponseStream* response, const long expression);
bool _razor_fromExpression(AsyncResponseStream* response, const char * expression);
bool _razor_fromExpression(AsyncResponseStream* response, const HTMLString expression);
bool _razor_fromExpression(AsyncResponseStream* response, const Printable& expression);

#endif

