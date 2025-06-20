#include <WiFi.h>
#include <WebServer.h>

#define RXD2 16
#define TXD2 17

// Remplacez par vos infos Wi-Fi AP
const char* ssid     = "BILLY_ESP32";
const char* password = "Rodolphe64!";

WebServer server(80);
bool controlEnabled = false;

// Votre page HTML complète en flash
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="fr">
<head>
  <meta charset="UTF-8">
  <title>Supervision Robot</title>
  <style>
    * { box-sizing: border-box; margin: 0; padding: 0; font-family: sans-serif; }
    body, html { width: 100%; height: 100%; }
    .container { display: flex; height: 100vh; }
    .section { flex: 1; padding: 10px; border-left: 1px solid #ddd;
      display: flex; flex-direction: column; align-items: center; }
    .section:first-child { border-left: none; }
    /* ===== 1. Contrôle BILLY ===== */
    .control { justify-content: center; }
    .control h2 { margin-bottom: 40px; font-size: 24px; }
    .control-toggle { display: flex; align-items: center; gap: 10px; margin-bottom: 30px; }
    .switch { position: relative; display: inline-block; width: 50px; height: 24px; }
    .switch input { opacity: 0; width: 0; height: 0; }
    .slider { position: absolute; cursor: pointer;
      top: 0; left: 0; right: 0; bottom: 0;
      background: #ccc; transition: .4s; border-radius: 24px; }
    .slider:before {
      position: absolute; content: "";
      height: 18px; width: 18px;
      left: 3px; bottom: 3px;
      background: white; transition: .4s; border-radius: 50%;
    }
    .switch input:checked + .slider { background: #4CAF50; }
    .switch input:checked + .slider:before { transform: translateX(26px); }

    .control-ui {
      display: flex; flex-direction: column; align-items: center;
      justify-content: space-between; width: 90%; max-width: 400px; flex: 1;
    }
    .control-ui button:disabled { background: #bbb; cursor: not-allowed; }
    .control-ui input:disabled  { background: #eee; cursor: not-allowed; }

    .control .grid {
      display: grid;
      grid-template-areas:
        ". up ."
        "left stop right"
        ". down .";
      grid-template-columns: 1fr 1fr 1fr;
      grid-template-rows:    1fr 1fr 1fr;
      grid-gap: 12px;
      width: 100%; flex: 1; max-height: 400px; margin-bottom: 20px;
    }
    .control .grid button {
      width: 100%; height: 100%;
      font-size: 2.5rem; border: none; border-radius: 8px;
      background: #eee; cursor: pointer; transition: background .2s;
    }
    .control .grid button:hover { background: #ddd; }
    .control .up    { grid-area: up; }
    .control .down  { grid-area: down; }
    .control .left  { grid-area: left; }
    .control .right { grid-area: right; }
    .control .stop  {
      grid-area: stop; background: #e74c3c; color: #fff;
    }
    .control .stop:hover { background: #c0392b; }

    .control .gyrophare {
      padding: 10px 20px; background: #f1c40f; border: none;
      border-radius: 6px; font-size: 18px; cursor: pointer;
      margin: 20px 0;
    }

    .control .speed-control {
      margin-bottom: 40px;
    }
    .control .speed-control input {
      width: 120px; padding: 8px; font-size: 18px;
    }
    .control .speed-control button {
      padding: 8px 16px; margin-left: 8px;
      font-size: 18px; cursor: pointer;
    }

    /* ===== 2. Supervision BILLY ===== */
    .state { justify-content: center; }
    .state h2 { margin-bottom: 40px; font-size: 24px; }
    .state .row {
      display: flex; justify-content: space-between;
      width: 100%; margin: 4px 0 16px;
    }
    .state .row .cell { flex: 1; text-align: center; }
    .state img {
      flex: 1; max-width: 100%; height: auto; object-fit: contain;
      margin: 30px 0;
    }
    .state .grid-info {
      display: flex; justify-content: center; gap: 40px; margin-top: 20px;
    }
    .state .grid-info .col { display: flex; flex-direction: column; }
    .state .grid-info .line { margin: 4px 0; font-size: 16px; }

    /* ===== 3. Journal & Carte ===== */
    .log { justify-content: flex-start; }
    .log h2 {
      margin-bottom: 40px; font-size: 24px; text-align: center;
    }
    .log-box {
      flex: 1; width: 100%; background: #f5f5f5; color: #333;
      padding: 10px; font-family: monospace; font-size: 14px;
      overflow-y: auto; margin-bottom: 10px; border: 1px solid #ccc;
      white-space: pre-wrap;
    }
    canvas {
      border: 1px solid #999; background: #fff; flex: 1;
    }
  </style>
</head>
<body>
  <div class="container">
    <!-- SECTION 1 : Contrôle BILLY -->
    <div class="section control">
      <h2>Contrôle BILLY</h2>
      <div class="control-toggle">
        <label class="switch">
          <input type="checkbox" id="control-switch"
                 onchange="toggleControl(this.checked)">
          <span class="slider"></span>
        </label>
        <span id="control-label">Contrôle désactivé</span>
      </div>
      <div class="control-ui">
        <div class="grid">
          <button class="up"    onclick="cmd('AV')">↑</button>
          <button class="left"  onclick="cmd('TG')">←</button>
          <button class="stop"  onclick="cmd('ST')">■</button>
          <button class="right" onclick="cmd('TD')">→</button>
          <button class="down"  onclick="cmd('RE')">↓</button>
        </div>
        <button class="gyrophare" onclick="cmd('GY')">Gyrophare</button>
        <div class="speed-control">
          <input type="number" id="speed" placeholder="Vitesse" />
          <button onclick="setSpeed()">Envoyer</button>
        </div>
      </div>
    </div>
    <!-- SECTION 2 : Supervision BILLY -->
    <div class="section state">
      <h2>Supervision BILLY</h2>
      <div class="row">
        <div class="cell">Statut : <strong><span id="robot-state">–</span></strong></div>
        <div class="cell">Batterie : <strong><span id="battery">–</span>%</strong></div>
      </div>
      <img src="billy.png" alt="Robot BILLY">
      <div class="grid-info">
        <div class="col">
          <div class="line">Cap. Gauche     : <span id="sensor-left">–</span> cm</div>
          <div class="line">Cap. Av-Gauche  : <span id="sensor-fl">–</span> cm</div>
          <div class="line">Cap. Av-Droite  : <span id="sensor-fr">–</span> cm</div>
          <div class="line">Cap. Droite     : <span id="sensor-right">–</span> cm</div>
        </div>
        <div class="col">
          <div class="line">Vitesse Moteur G: <span id="motor-left">–</span> m/s</div>
          <div class="line">Vitesse Moteur D: <span id="motor-right">–</span> m/s</div>
          <div class="line">Distance        : <span id="distance">–</span> m</div>
          <div class="line">Durée           : <span id="duration">–</span> s</div>
        </div>
      </div>
    </div>
    <!-- SECTION 3 : Journal & Carte -->
    <div class="section log">
      <h2>Logs Billy</h2>
      <div id="log" class="log-box"></div>
      <h2>BILLY Map</h2>
      <canvas id="map" width="300" height="300"></canvas>
    </div>
  </div>

  <script>
    window.addEventListener('load', () => {
      toggleControl(false);
      if (window.EventSource) {
        const es = new EventSource('/events');
        es.onmessage = e => {
          const d = JSON.parse(e.data);
          document.getElementById('robot-state').innerText  = d.state;
          document.getElementById('battery').innerText      = d.battery;
          document.getElementById('sensor-left').innerText  = d.sensors.left;
          document.getElementById('sensor-fl').innerText    = d.sensors.frontLeft;
          document.getElementById('sensor-fr').innerText    = d.sensors.frontRight;
          document.getElementById('sensor-right').innerText = d.sensors.right;
          document.getElementById('motor-left').innerText   = d.motors.left;
          document.getElementById('motor-right').innerText  = d.motors.right;
          document.getElementById('distance').innerText     = d.distance;
          document.getElementById('duration').innerText     = d.duration;
          appendLog(d.log);
          plotPoint(d.pos.x, d.pos.y);
        };
      }
      initDemo();
    });

    function toggleControl(enabled) {
      fetch(`/toggleControl?enabled=${enabled}`).catch(console.error);
      document.querySelectorAll('.control-ui button, .control-ui input')
              .forEach(el => el.disabled = !enabled);
      document.getElementById('control-label')
              .innerText = enabled ? 'Contrôle activé' : 'Contrôle désactivé';
    }
    function cmd(c)     { fetch(`/cmd?c=${c}`).catch(console.error); }
    function setSpeed() {
      const v = document.getElementById('speed').value;
      fetch(`/setSpeed?s=${encodeURIComponent(v)}`).catch(console.error);
    }
    function appendLog(txt) {
      const l = document.getElementById('log');
      l.innerText += txt + "\n";
      l.scrollTop = l.scrollHeight;
    }
    const canvas = document.getElementById('map'),
          ctx    = canvas.getContext('2d');
    let last = null;
    function plotPoint(x,y){
      const sx = x*(canvas.width/10),
            sy = canvas.height - y*(canvas.height/10);
      if(last){
        ctx.beginPath();
        ctx.moveTo(last.x,last.y);
        ctx.lineTo(sx,sy);
        ctx.strokeStyle='#e74c3c';
        ctx.lineWidth=2;
        ctx.stroke();
      }
      last={x:sx,y:sy};
      ctx.fillStyle='#3498db';
      ctx.beginPath();
      ctx.arc(sx,sy,3,0,2*Math.PI);
      ctx.fill();
    }
    const demoPoints=[{x:1,y:1},{x:9,y:1},{x:9,y:9},{x:1,y:9},{x:1,y:5}];
    function initDemo(){
      ctx.clearRect(0,0,canvas.width,canvas.height);
      ctx.strokeStyle='#ccc'; ctx.lineWidth=1;
      ctx.strokeRect(0,0,canvas.width,canvas.height);
      last=null;
      demoPoints.forEach(p=>plotPoint(p.x,p.y));
    }
  </script>
</body>
</html>
)rawliteral";

// Gestion des routes
void handleNotFound(){
  Serial.printf("404 %s\n", server.uri().c_str());
  server.send(404, "text/plain", "Not found");
}
void handleRoot(){
  Serial.println("GET / -> index.html");
  server.send_P(200, "text/html", INDEX_HTML);
}
void handleToggle(){
  if(!server.hasArg("enabled")){
    server.send(400, "text/plain", "Missing enabled");
    return;
  }
  controlEnabled = (server.arg("enabled") == "true");
  Serial.print(controlEnabled ? "{\n" : "}\n");
  Serial2.print(controlEnabled ? "{\n" : "}\n");
  server.send(200, "text/plain", "OK");
}
void handleCmd(){
  if(!server.hasArg("c")){
    server.send(400, "text/plain", "Missing cmd");
    return;
  }
  if(!controlEnabled){
    server.send(403, "text/plain", "Control disabled");
    return;
  }
  String c = server.arg("c"), out;
  if      (c=="AV") out="*AV";
  else if (c=="RE") out="*RE";
  else if (c=="TG") out="*TG";
  else if (c=="TD") out="*TD";
  else if (c=="ST") out="*ST";
  else if (c=="GY") out="*GY";
  else {
    server.send(400, "text/plain", "Unknown cmd");
    return;
  }
  Serial.println(out);
  Serial2.println(out);
  server.send(200, "text/plain", "OK");
}
void handleSpeed(){
  if(!server.hasArg("s")){
    server.send(400, "text/plain", "Missing speed");
    return;
  }
  if(!controlEnabled){
    server.send(403, "text/plain", "Control disabled");
    return;
  }
  String v = server.arg("s");
  String out = "*CV"+v;
  Serial.println(out);
  Serial2.println(out);
  server.send(200, "text/plain", "OK");
}
void handleEvents(){
  // Exemple statique, à adapter si besoin
  String s = "data: {";
  s += "\"state\":\"OK\",";
  s += "\"battery\":100,";
  s += "\"sensors\":{ \"left\":0,\"frontLeft\":0,\"frontRight\":0,\"right\":0 },";
  s += "\"motors\":{ \"left\":0,\"right\":0 },";
  s += "\"distance\":0,";
  s += "\"duration\":0,";
  s += "\"pos\":{ \"x\":0,\"y\":0 },";
  s += "\"log\":\"\"";
  s += "}\n\n";
  server.send(200, "text/event-stream", s);
}

void setup(){
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  delay(1000);
  Serial.println("\n=== Démarrage BILLY ESP32 ===");
  // Mode point d'accès
  WiFi.mode(WIFI_AP);
  if(WiFi.softAP(ssid, password)){
    Serial.printf("AP démarré : %s\n", ssid);
    Serial.print("IP AP     : ");
    Serial.println(WiFi.softAPIP());
  } else {
    Serial.println("Échec softAP !");
  }
  // Routes
  server.onNotFound(handleNotFound);
  server.on( "/",              HTTP_GET, handleRoot);
  server.on( "/toggleControl", HTTP_GET, handleToggle);
  server.on( "/cmd",           HTTP_GET, handleCmd);
  server.on( "/setSpeed",      HTTP_GET, handleSpeed);
  server.on( "/events",        HTTP_GET, handleEvents);
  server.begin();
  Serial.println("HTTP server démarré");
}

void loop(){
  server.handleClient();
}
