void index_cshtml(AsyncWebServerRequest *request) {
  AsyncResponseStream *response = request->beginResponseStream("text/html");
  response->addHeader("Cache-Control", "no-cache");
  File file = SPIFFS.open("/index.cshtml", "r");
  _razor_fromFile(response, &file, 0, 3);
 
    onIndexRequest(request);

  _razor_fromFile(response, &file, 39, 504);
  _razor_fromExpression(response, (settings.page_title));

  _razor_fromFile(response, &file, 526, 988);
  _razor_fromExpression(response, (settings.page_title));

  _razor_fromFile(response, &file, 1010, 1475);
  _razor_fromExpression(response, (settings.led_red ? "checked" : ""));

  _razor_fromFile(response, &file, 1511, 1720);
  _razor_fromExpression(response, (settings.led_green ? "checked" : ""));

  _razor_fromFile(response, &file, 1758, 1964);
  _razor_fromExpression(response, (settings.led_blue ? "checked" : ""));

  _razor_fromFile(response, &file, 2001, 3914);
  file.close(); 
  request->send(response);
}
void razor_setup(AsyncWebServer *server) {
  server->on("/index.cshtml", index_cshtml);
}
