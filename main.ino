AsyncWebServer server(80);

#define RECV_PIN 35
#define SEND_PIN 2
#define CAPTURE_BUFFER_SIZE 1024
#define TIMEOUT 50

IRsend irsend(SEND_PIN);
IRrecv irrecv(RECV_PIN, CAPTURE_BUFFER_SIZE, TIMEOUT, true);

String histIRcode;

void OTASetup()
{
    Serial.println("OTASetup");
    server.on("/", HTTP_GET, handleIndex);
    server.on("/ir", HTTP_GET, handleIRGet);
    server.onRequestBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        String url = request->url();
        if (url == "/ir")
            handleIRPost(request, data);
    });
    server.onNotFound(handleNotFound);
    server.begin();

    irsend.begin();
    irrecv.enableIRIn();
}

void handleIndex(AsyncWebServerRequest *request)
{
    // irrecv.disableIRIn();
    String s = "<html lang=\"en\"><head><meta charset=\"utf-8\"/><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><title>IR Console</title><link rel=\"stylesheet\" href=\"https://cdnjs.cloudflare.com/ajax/libs/twitter-bootstrap/3.3.7/css/bootstrap.min.css\" /><script src=\"https://cdnjs.cloudflare.com/ajax/libs/jquery/3.2.1/jquery.min.js\"></script><script src=\"https://cdnjs.cloudflare.com/ajax/libs/twitter-bootstrap/3.3.7/js/bootstrap.min.js\"></script></head><body><div class=\"container\"><div class=\"row\"><div class=\"col-md-12\"><h1>IR console";
    s += "</h1><p>IP address: ";
    s += String(ETH.localIP()[0]) + "." + String(ETH.localIP()[1]) + "." + String(ETH.localIP()[2]) + "." + String(ETH.localIP()[3]);
    s += "</p><hr><div class=\"form-group\"><textarea class=\"form-control\" id=\"message\" rows=\"10\"></textarea></div><button class=\"btn btn-primary\" id=\"get\">GET</button> <button class=\"btn btn-success\" id=\"post\">POST</button></div></div><script>var xhr = new XMLHttpRequest();var textarea = document.getElementById(\"message\");document.getElementById(\"get\").addEventListener(\"click\", function () {xhr.open('GET', '/ir', true);xhr.onreadystatechange = function() {if(xhr.readyState == 4) {textarea.value =xhr.responseText;}};xhr.send(null);});document.getElementById(\"post\").addEventListener(\"click\", function () {data = textarea.value;xhr.open('POST', '/ir', true);xhr.onreadystatechange = function() {if(xhr.readyState == 4) {alert(xhr.responseText);}};xhr.send(data);});</script></body></html>";
    request->send(200, "text/html", s);
}

void handleIRGet(AsyncWebServerRequest *request)
{
    String s = "{\"data\":{\"format\":\"raw\",\"freq\":38,\"signal\":[";
    s += histIRcode;
    s += "]}}";

    request->send(200, "text/html", s);
}

bool handleIRPost(AsyncWebServerRequest *request, uint8_t *datas)
{
    String s = "";
    StaticJsonDocument<4000> root;
    DeserializationError error = deserializeJson(root, (const char *)datas);

    if (!error)
    {
        int freq = root["data"]["freq"];
        int d_size = root["data"]["signal"].size();
        uint16_t rawData[d_size];
        for (int i = 0; i < d_size; i++)
        {
            rawData[i] = root["data"]["signal"][i];
        }
        irrecv.disableIRIn();
        irsend.sendRaw(rawData, d_size, freq);
        irrecv.enableIRIn();
        s += "ok";
    }
    else
    {
        s += "Invalid JSON format";
    }

    request->send(200, "text/html", s);
}

void handleNotFound(AsyncWebServerRequest *request)
{
    // irrecv.disableIRIn();
    request->send(404, "text/plain", "Not found");
}

void OTALoop()
{
    decode_results results; // Somewhere to store the results
    if (irrecv.decode(&results))
    {                               // Grab an IR code
        dumpCode(&results);         // Blank line between entries
        Serial.println(histIRcode); // Blank line between entries
        irrecv.resume();            // Prepare for the next value
    }
}

void dumpCode(decode_results *results)
{
    // Dump data
    histIRcode = "";
    for (int i = 1; i < results->rawlen; i++)
    {
        histIRcode += results->rawbuf[i] * 2;
        // Serial.print(results->rawbuf[i] * 50, DEC);
        if (i < results->rawlen - 1)
            histIRcode += ","; // ',' not needed on last one
    }
}

void OTAStop()
{
    irrecv.disableIRIn();
}
