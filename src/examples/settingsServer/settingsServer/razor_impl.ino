void index_cshtml(AsyncWebServerRequest *request) {
  AsyncResponseStream *response = request->beginResponseStream("text/html");
  response->addHeader("Cache-Control", "no-cache");
  File file = SPIFFS.open("/index.cshtml", "r");
  _razor_fromFile(response, &file, 0, 303);
  _razor_fromExpression(response, (settings.page_title));

  _razor_fromFile(response, &file, 325, 1253);
  _razor_fromExpression(response, (settings.led_red ? "checked" : ""));

  _razor_fromFile(response, &file, 1289, 1526);
  _razor_fromExpression(response, (settings.led_green ? "checked" : ""));

  _razor_fromFile(response, &file, 1564, 1798);
  _razor_fromExpression(response, (settings.led_blue ? "checked" : ""));

  _razor_fromFile(response, &file, 1835, 2670);
  file.close(); 
  request->send(response);
}
void settings_cshtml(AsyncWebServerRequest *request) {
  AsyncResponseStream *response = request->beginResponseStream("text/html");
  response->addHeader("Cache-Control", "no-cache");
  File file = SPIFFS.open("/settings.cshtml", "r");
  _razor_fromFile(response, &file, 0, 3);
 
    onSettingsRequest(request);

  _razor_fromFile(response, &file, 42, 344);
  _razor_fromExpression(response, (settings.page_title));

  _razor_fromFile(response, &file, 366, 912);
  _razor_fromExpression(response, (settings.wifi_sta ? "" : "checked"));

  _razor_fromFile(response, &file, 949, 1153);
  _razor_fromExpression(response, (settings.wifi_sta ? "checked" : ""));

  _razor_fromFile(response, &file, 1190, 1469);
  _razor_fromExpression(response, (settings.ssid_name));

  _razor_fromFile(response, &file, 1490, 2502);
  _razor_fromExpression(response, (settings.emergency_ssid_name));

  _razor_fromFile(response, &file, 2533, 3190);
  _razor_fromExpression(response, (settings.page_title));

  _razor_fromFile(response, &file, 3212, 3670);
  _razor_fromExpression(response, (settings.led_red ? "checked" : ""));

  _razor_fromFile(response, &file, 3706, 3925);
  _razor_fromExpression(response, (settings.led_green ? "checked" : ""));

  _razor_fromFile(response, &file, 3963, 4179);
  _razor_fromExpression(response, (settings.led_blue ? "checked" : ""));

  _razor_fromFile(response, &file, 4216, 4788);
  _razor_fromExpression(response, settings.light_min);

  _razor_fromFile(response, &file, 4807, 5049);
  _razor_fromExpression(response, settings.light_max);

  _razor_fromFile(response, &file, 5068, 5395);
  _razor_fromExpression(response, settings.email);

  _razor_fromFile(response, &file, 5410, 6336);
  file.close(); 
  request->send(response);
}
void razor_setup(AsyncWebServer *server) {
  server->on("/index.cshtml", index_cshtml);
  server->on("/settings.cshtml", settings_cshtml);
}
