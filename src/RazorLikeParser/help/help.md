Usage
-----

`dotnet RazorLikeParser.dll <sketch folder>`

* `<sketch folder>`  
  The folder where the main .ino file is located.


All `<sketch folder>/data/*.cshtml` files will be parsed. Results will be placed in:

* `<sketch folder>/razor_def.ino`
  This file will contain the fixed part of the output
* `<sketch folder>/razor_impl.ino`
  This file will contain the parsed result.

Call `razor_setup(&server)` from your `setup()` to initialize these pages.

Do not forget to upload the `data` folder to the Flash/SPIFFS of the ESP8266.
