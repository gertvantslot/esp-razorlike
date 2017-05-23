/*
MIT License

Copyright(c) 2017 Gert van 't Slot - gert@vantslot.be

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

using System;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Resources;
using System.Text;
using System.Threading;

namespace RazorLikeParser
{
    class Program
    {
        static void Main(string[] args) {
            Console.WriteLine("RazorLikeParser v0.0 - by GvS");

            if (args.Length == 0 || string.IsNullOrWhiteSpace(args[0])) {
                ShowHelp("help");
                return;
            }

            var folder = args[0];

            if (!Directory.Exists(folder)) {
                ShowHelp("folder_does_not_exist");
                ShowHelp("help");
                return;
            }

            var dataFolder = Path.Combine(folder, "data");
            if (!Directory.Exists(dataFolder)) {
                ShowHelp("datafolder_does_not_exist");
                ShowHelp("help");
                return;
            }

            // Create razor_def.ino file
            CreateStaticFiles(folder);

            // Create dynamic files
            var files = Directory.GetFiles(dataFolder, "*.cshtml");
            using (var razor = File.CreateText(Path.Combine(folder, "razor_impl.ino"))) {
                foreach (var file in files) {
                    process(razor, file);
                }

                razor.WriteLine("void razor_setup(AsyncWebServer *server) {");

                foreach (var file in files) {
                    var fName = Path.GetFileNameWithoutExtension(file);
                    razor.WriteLine($"  server->on(\"/{Path.GetFileName(file)}\", {fName}_cshtml);");
                }

                razor.WriteLine("}");
            }
        }

            static void ShowHelp(string subject) {
            var am = Assembly.GetEntryAssembly();
            var location = $"{am.GetName().Name}.help.{subject}.md";
            using (var s = am.GetManifestResourceStream(location))
            using (var rdr = new StreamReader(s))  {
                Console.WriteLine(rdr.ReadToEnd());
            }
        }

        static void CreateStaticFiles(string folder) {
            var am = Assembly.GetEntryAssembly();
            var location = $"{am.GetName().Name}.ino.";
            foreach (var ino in am.GetManifestResourceNames()
                .Where(s => s.StartsWith(location))) {
                var destName = ino.Substring(location.Length);
                var destPath = Path.Combine(folder, destName);
                using (var dest = File.Create(destPath))
                using (var source = am.GetManifestResourceStream(ino)) {
                    source.CopyTo(dest);
                }
                Console.WriteLine($"* {destName}");
            }
        }


        static byte[] content;
        static int index;
        static long startClientBlock = 0;

        private static bool EOF {
            get {
                return index >= content.Length;
            }
        }
        private static char nextChar() {
            return (char)content[index++];
        }

        private static char peek() {
            return (char)content[index];
        }

        static int _clientEnd = 0;
        static int _clientStart = 0;

        static void registerClientStart(StreamWriter razor) {
            if (_clientStart == -1) {
                _clientStart = index;
                return;
            }
            if (index == _clientEnd) {
                // Does not change a thing
                return;
            }
            _clientStart = index;
        }

        static void registerClientEnd(StreamWriter razor, int offset = 0, bool force = false) {
            _clientEnd = index + offset;
            if (force && _clientStart != -1) {
                razor.WriteLine($"  _razor_fromFile(response, &file, {_clientStart}, {_clientEnd});");
                _clientStart = -1;
            }
        }

        static void process(StreamWriter razor, string fileName) {
            _clientEnd = _clientStart = 0;
            razor.WriteLine($"void {Path.GetFileNameWithoutExtension(fileName)}_cshtml(AsyncWebServerRequest *request) {{");
            razor.WriteLine($"  AsyncResponseStream *response = request->beginResponseStream(\"text/html\");");
            razor.WriteLine($"  response->addHeader(\"Cache-Control\", \"no-cache\");");
            razor.WriteLine($"  File file = SPIFFS.open(\"/{Path.GetFileName(fileName)}\", \"r\");");

            content = File.ReadAllBytes(fileName);
            index = 0;

            while (!EOF) {
                parseClientMode(razor);
            }

            // Write end block
            if (startClientBlock < index) {
                registerClientEnd(razor, force: true);
            }

            razor.WriteLine("  file.close(); ");
            razor.WriteLine("  request->send(response);");
            razor.WriteLine("}");
        }

        static void parseClientMode(StreamWriter razor) {
            string tag = null;
            string endTag = null;
            bool tagReading = false;
            bool endTagReading = false;
            registerClientStart(razor);
            startClientBlock = index;

            while (!EOF) {
                var c = nextChar();

                if (tagReading) {
                    if (char.IsLetter(c)) {
                        tag += c;
                    } else {
                        tagReading = false;
                    }
                }

                if (c == '/' && peek() == '>') {
                    // Single ended tag
                    nextChar();
                    // Write file to client
                    //razor.WriteLine();
                    //razor.WriteLine($"  sendFromFile(&file, &server, {startClientBlock}, {index});");
                    //startClientBlock = index;
                    registerClientEnd(razor);
                    return;
                }

                if (c == '<' && peek() == '/') {
                    nextChar();
                    endTagReading = true;
                    endTag = "";
                    continue;
                }

                if (endTagReading) {
                    if (!char.IsLetterOrDigit(c)) {
                        endTagReading = false;
                        if (endTag == tag) {
                            // Write file to client
                            //razor.WriteLine($"  sendFromFile(&file, &server, {startClientBlock}, {index});");
                            startClientBlock = index;
                            registerClientEnd(razor);
                            return;
                        }
                        continue;
                    }
                    endTag += c;
                }

                if (!tagReading && c == '<' && string.IsNullOrEmpty(tag)) {
                    tagReading = true;
                    tag = "";
                    continue;
                }
                if (c == '@') {

                    c = peek();
                    // Enter server mode
                    registerClientEnd(razor, offset: -1, force: true);
                    if (c != '{') {
                        parseExpression(razor);
                        startClientBlock = index;
                        registerClientStart(razor);
                    } else {
                        parseServerMode(razor);
                        startClientBlock = index;
                        registerClientStart(razor);
                    }


                }
            }
        }

        static void parseServerMode(StreamWriter razor) {
            var depth = 0;
            bool startOfStatement = true;
            if (peek() == '{') {
                nextChar();
            }
            while (!EOF) {
                var c = nextChar();
                if (startOfStatement && c == '<') {
                    // back to client mode;
                    razor.WriteLine();
                    index--;
                    parseClientMode(razor);
                    continue;
                }
                if (c == '{') depth++;
                if (startOfStatement && c == '}') {
                    depth--;
                    if (depth < 0) {
                        razor.WriteLine();

                        return;
                    }
                }
                registerClientEnd(razor, force: true);
                razor.Write(c);
                if (char.IsLetterOrDigit(c)) {
                    startOfStatement = false;
                }
                if (c == ';' || c == '{' || c == '\n') {
                    startOfStatement = true;
                }
            }
        }

        static void parseExpression(StreamWriter razor) {
            var expression = new StringBuilder();

            while (!EOF) {
                var c = nextChar();
                if (char.IsWhiteSpace(c) || char.IsControl(c) || c == '<' || c == '"') {
                    razor.WriteLine($"  _razor_fromExpression(response, {expression.ToString()});");
                    razor.WriteLine();
                    index--;
                    return;
                }
                if (c == '(') {
                    expression.Append(parseBlock('(', ')'));
                } else {
                    expression.Append(c);
                }
            }

        }

        static string parseBlock(char blockStart, char blockEnd) {
            var result = new StringBuilder(blockStart.ToString());
            while (!EOF) {
                var c = nextChar();
                if (c == blockEnd) {
                    result.Append(c);
                    return result.ToString();
                }
                if (c == '(') {
                    result.Append(parseBlock('(', ')'));
                } else if (c == '{') {
                    result.Append(parseBlock('{', '}'));
                } else {
                    result.Append(c);
                }
            }
            return result.ToString();
        }
    }
}