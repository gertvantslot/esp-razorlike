void index_cshtml(AsyncWebServerRequest *request) {
  AsyncResponseStream *response = request->beginResponseStream("text/html");
  response->addHeader("Cache-Control", "no-cache");
  File file = SPIFFS.open("/index.cshtml", "r");
  _razor_fromFile(response, &file, 0, 38);

  // Handle post
  handlePost(request);

  _razor_fromFile(response, &file, 85, 1709);
  _razor_fromExpression(response, light());

  _razor_fromFile(response, &file, 1717, 1884);
  _razor_fromExpression(response, temperature());

  _razor_fromFile(response, &file, 1898, 2281);
  _razor_fromExpression(response, red());

  _razor_fromFile(response, &file, 2287, 2916);
  _razor_fromExpression(response, green());

  _razor_fromFile(response, &file, 2924, 3551);
  _razor_fromExpression(response, blue());

  _razor_fromFile(response, &file, 3558, 3943);
 for(int i = 0; i < 10; i++) {
             
  _razor_fromFile(response, &file, 3990, 4044);
  _razor_fromExpression(response, i);

  _razor_fromFile(response, &file, 4046, 4068);

            }
          
  _razor_fromFile(response, &file, 4095, 4310);
  file.close(); 
  request->send(response);
}
void razor_setup(AsyncWebServer *server) {
  server->on("/index.cshtml", index_cshtml);
}
