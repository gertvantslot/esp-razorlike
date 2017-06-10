# esp-razorlike

Code generator, generates code for a Web-server on the ESP8266 powered from Html pages with Razor-like syntax.

## Purpose

Server a Web page from an ESP8266/NodeMcu:

* Use HTML as template
* Dynamic HTML generation
* Minimal program size
* Minimal runtime size
* Runtime performance

### Overview of usage cycle

1. Create your main .ino sketch/project, setup Wifi and ESPAsyncWebServer.
2. Create .cshtml file's (in the data folder) with C++ mixins
2. Run parser. This will create two .ino files that will be compiled together with your main sketch.
3. Upload data folder to the SPIFFS filesystem.
5. Update

## Html Syntax
It uses a simplified version of the ASP.Net MVC Razor syntax.

Html code is send to the client. To insert the result of server (ESP8266) code, use an @.

    <p>The temperature is @temperature()</p>

The above code will insert the result from a call the the `temperature` method.

Output that is of type `char *` will be Html encoded, for your safety. If you need your `char *` to be outputed
as raw Html, use `@Raw( Your_code() )`.

To execute C++ code on the server, use `@{ // C++ code here }`. To switch
back to HTML code, start a new tag:

    <div class="row">
    @{
      for(int i = 0; i < 3; i++) {
         int led = digitalRead(i);
         <div class="col-md-4">
           <p>Status of led #<strong>@i</strong> = <strong>@led</strong></p>
         </div>
      }
    }
    </div>

This will create a new row, with 3 columns. In each column the result of a digitalRead will be displayed.

## Generated code

The generated code will read the .cshtml file from the SPIFFS filesystem, and copy this
directly to the output stream, mixed with the execution of
C++ code.

The above example could be parsed into something like:

    _razor_fromFile(response, &file, 3558, 3943);
    for(int i = 0; i < 3; i++) {
        int led = digitalRead(i);
        _razor_fromFile(response, &file, 3990, 4044);
        _razor_fromExpression(response, i);
        _razor_fromFile(response, &file, 4070, 4085);
        _razor_fromExpression(response, led);
        _razor_fromFile(response, &file, 4087, 4101);
    }

## Usage

Download the .zip file and extract to a folder of choice.

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


### Example

See folder: src/Examples

