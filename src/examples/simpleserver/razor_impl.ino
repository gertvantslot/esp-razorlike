void index_cshtml(AsyncWebServerRequest *request) {
  AsyncResponseStream *response = request->beginResponseStream("text / html");
  response->addHeader("Cache - Control", "no - cache");
  File file = SPIFFS.open("/index.cshtml", "r");
  _razor_fromFile(response, &file, 0, 38);

  // Handle post
  handlePost(request);

  _razor_fromFile(response, &file, 85, 1630);
  _razor_fromExpression(response, light());

  _razor_fromFile(response, &file, 1638, 1805);
  _razor_fromExpression(response, temperature());

  _razor_fromFile(response, &file, 1819, 1933);
 for(int i = 0; i < 10; i++) {
             
  _razor_fromFile(response, &file, 1980, 2034);
  _razor_fromExpression(response, i);

  _razor_fromFile(response, &file, 2036, 2058);

            }
          
  _razor_fromFile(response, &file, 2085, 2292);
  file.close(); 
  request->send(response);
}
void razor_setup(AsyncWebServer *server) {
  server->on("/index.cshtml", index_cshtml);
}
