#include <WiFi.h>
#include <WebServer.h>

const char* apSSID = "BILLY_ESP32_";
const char* apPass = "Rodolphe64!";

WebServer server(80);
#define RXD2 16
#define TXD2 17

const char HTML_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html><head><meta charset="UTF-8"><title>Télécommande</title>
<style>
 body { display:flex; height:100vh; margin:0; justify-content:center; align-items:center; }
 .grid{display:grid;grid-template-areas:". up .""left stop right"". down .";grid-gap:20px;}
 button{width:80px;height:80px;font-size:1.2em;}
 .up{grid-area:up;}.down{grid-area:down;}.left{grid-area:left;}
 .right{grid-area:right;}.stop{grid-area:stop;background:#f00;color:#fff;}
</style>
</head>
<body>
 <div class="grid">
   <button class="up"    onclick="cmd('A')">↑</button>
   <button class="left"  onclick="cmd('G')">←</button>
   <button class="stop"  onclick="cmd('S')">•</button>
   <button class="right" onclick="cmd('D')">→</button>
   <button class="down"  onclick="cmd('R')">↓</button>
 </div>
 <script>
   function cmd(c) {
     fetch('/cmd?c='+c, {cache:'no-store'})
       .then(r=>r.text()).then(t=>console.log('resp',t));
   }
 </script>
</body>
</html>
)rawliteral";

void setup(){
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  WiFi.softAP(apSSID, apPass);
  Serial.printf("AP %s / %s  IP=%s\n", apSSID, apPass, WiFi.softAPIP().toString().c_str());
  server.on("/", [](){ server.send_P(200,"text/html",HTML_PAGE); });
  server.on("/cmd", [](){
    char c = server.arg("c")[0];
    Serial.printf("%c\n", c);
    Serial2.write(c);
    server.send(200,"text/plain","OK");
  });
  server.begin();
}

void loop(){
  server.handleClient();
}
