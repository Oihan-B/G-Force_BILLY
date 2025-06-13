#include <WiFi.h>
#include <WebServer.h>

const char* apSSID = "BILLY_ESP32_";
const char* apPass = "Rodolphe64!";  

WebServer server(80);

float lastVitesse  = 0.0;
float lastPosition = 0.0;

#define RXD 16 
#define TXD 17   
String rxBuf;

const char HTML_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html>
<head><meta charset="UTF-8"><title>Telemetrie</title></head>
<body>
  <h1>Télémetrie Teensy → ESP32</h1>
  <div>Vitesse : <span id="vit">–</span> m/s</div>
  <div>Position : <span id="pos">–</span> m</div>
  <script>
    function refresh() {
      fetch('/data')
        .then(r=>r.json())
        .then(j=>{
          document.getElementById('vit').textContent = j.VIT.toFixed(1);
          document.getElementById('pos').textContent = j.POS.toFixed(1);
        });
    }
    setInterval(refresh, 2000);
    window.onload = refresh;
  </script>
</body>
</html>
)rawliteral";

void setup() {

  Serial.begin(115200);
  while(!Serial){}  

  Serial2.begin(115200, SERIAL_8N1, RXD, TXD);
  Serial.println("UART2 initialisé à 115200 bauds sur RX=3 / TX=1 (SERIAL_8N1)");

  WiFi.softAP(apSSID, apPass);
  IPAddress ip = WiFi.softAPIP();
  Serial.printf("AP démarré : SSID=\"%s\"  PASS=\"%s\"  IP=%s\n",
                apSSID, apPass, ip.toString().c_str());

  server.on("/", [](){
    server.send_P(200, "text/html", HTML_PAGE);
  });
  server.on("/data", [](){
    String j = "{";
    j += "\"VIT\":" + String(lastVitesse,1) + ",";
    j += "\"POS\":" + String(lastPosition,1);
    j += "}";
    server.send(200, "application/json", j);
  });
  server.begin();
  Serial.println("Serveur HTTP démarré sur le port 80");
}

void loop() {

  while (Serial2.available()) {
    char c = Serial2.read();
    if (c == '\n') {
      int idx = 0;
      while ((idx = rxBuf.indexOf('$', idx)) >= 0) {
        int h = rxBuf.indexOf('#', idx);
        if (h < 0) break;
        String name = rxBuf.substring(idx+1, h);
        int next = rxBuf.indexOf('$', h);
        String val = (next > 0)
                     ? rxBuf.substring(h+1, next)
                     : rxBuf.substring(h+1);
        float f = val.toFloat();
        if (name == "VIT")  lastVitesse  = f;
        if (name == "POS")  lastPosition = f;
        idx = h+1;
      }
      rxBuf = "";
    } else {
      rxBuf += c;
      if (rxBuf.length() > 200) rxBuf.remove(0, rxBuf.length()-200);
    }
  }

  server.handleClient();
}
