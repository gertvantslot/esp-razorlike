void index_cshtml(AsyncWebServerRequest *request) {
  AsyncResponseStream *response = request->beginResponseStream("text/html");
  response->addHeader("Cache-Control", "no-cache");
  File file = SPIFFS.open("/index.cshtml", "r");
  _razor_fromFile(response, &file, 0, 3);
 
    onIndexRequest(request);

  _razor_fromFile(response, &file, 39, 504);
  _razor_fromExpression(response, (settings.page_title));

  _razor_fromFile(response, &file, 526, 997);
  _razor_fromExpression(response, (settings.page_title));

  _razor_fromFile(response, &file, 1019, 1484);
  _razor_fromExpression(response, (settings.led_red ? "checked" : ""));

  _razor_fromFile(response, &file, 1520, 1729);
  _razor_fromExpression(response, (settings.led_green ? "checked" : ""));

  _razor_fromFile(response, &file, 1767, 1973);
  _razor_fromExpression(response, (settings.led_blue ? "checked" : ""));

  _razor_fromFile(response, &file, 2010, 2594);
  _razor_fromExpression(response, settings.light_min);

  _razor_fromFile(response, &file, 2613, 2855);
  _razor_fromExpression(response, settings.light_max);

  _razor_fromFile(response, &file, 2874, 3201);
  _razor_fromExpression(response, settings.email);

  _razor_fromFile(response, &file, 3216, 4377);
  file.close(); 
  request->send(response);
}
void razor_setup(AsyncWebServer *server) {
  server->on("/index.cshtml", index_cshtml);
}
