void index_cshtml(AsyncWebServerRequest *request) {
  AsyncResponseStream *response = request->beginResponseStream("text/html");
  response->addHeader("Cache-Control", "no-cache");
  File file = SPIFFS.open("/index.cshtml", "r");
  _razor_fromFile(response, &file, 0, 3);
 
    onIndexRequest(request);

  _razor_fromFile(response, &file, 39, 341);
  _razor_fromExpression(response, (settings.page_title));

  _razor_fromFile(response, &file, 363, 909);
  _razor_fromExpression(response, (settings.wifi_sta ? "" : "checked"));

  _razor_fromFile(response, &file, 946, 1150);
  _razor_fromExpression(response, (settings.wifi_sta ? "checked" : ""));

  _razor_fromFile(response, &file, 1187, 1466);
  _razor_fromExpression(response, (settings.ssid_name));

  _razor_fromFile(response, &file, 1487, 2499);
  _razor_fromExpression(response, (settings.emergency_ssid_name));

  _razor_fromFile(response, &file, 2530, 3187);
  _razor_fromExpression(response, (settings.page_title));

  _razor_fromFile(response, &file, 3209, 3667);
  _razor_fromExpression(response, (settings.led_red ? "checked" : ""));

  _razor_fromFile(response, &file, 3703, 3922);
  _razor_fromExpression(response, (settings.led_green ? "checked" : ""));

  _razor_fromFile(response, &file, 3960, 4176);
  _razor_fromExpression(response, (settings.led_blue ? "checked" : ""));

  _razor_fromFile(response, &file, 4213, 4785);
  _razor_fromExpression(response, settings.light_min);

  _razor_fromFile(response, &file, 4804, 5046);
  _razor_fromExpression(response, settings.light_max);

  _razor_fromFile(response, &file, 5065, 5392);
  _razor_fromExpression(response, settings.email);

  _razor_fromFile(response, &file, 5407, 6333);
  file.close(); 
  request->send(response);
}
void razor_setup(AsyncWebServer *server) {
  server->on("/index.cshtml", index_cshtml);
}
