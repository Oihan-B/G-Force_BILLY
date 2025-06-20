#include <WiFi.h>
#include <WebServer.h>

#define RXD2 16
#define TXD2 17

// === VOTRE AP WI-FI ===
const char* ssid     = "BILLY_ESP32";
const char* password = "Rodolphe64!";

// Intervalle d’actualisation du site (en ms)
const unsigned long actualisationSiteWeb = 4000;

WebServer server(80);
bool controlEnabled = false;

// === VARIABLES TÉLÉMÉTRIE PARSÉES ===
int    robotState    = 0;
float  vitD=0, vitG=0;
float  posX=0, posY=0;
float  cap_CG=0, cap_AG=0, cap_AD=0, cap_CD=0;
int    etatGyro      = 0;
float  dist=0, dureeMission=0, dureeTotal=0;
String lastLog       = "";

// Buffer temporaire de lecture série
String serialBuf;

// =============================================================================
// === FONCTIONS DE LECTURE & PARSING DU SERIAL 2 (Teensy) ====================
// =============================================================================
float getFloat(const String& s, const char* key){
  int i = s.indexOf(key);
  if(i<0) return 0;
  i += strlen(key);
  int j = s.indexOf('$', i);
  if(j<0) j = s.length();
  return s.substring(i,j).toFloat();
}
int getInt(const String& s, const char* key){
  return (int)getFloat(s,key);
}

// Parse une ligne complète, mise à jour des variables globales
void parseLine(const String& line) {
  robotState    = getInt(line, "$ETATROBOT#");
  vitD          = getFloat(line, "$VITD#");
  vitG          = getFloat(line, "$VITG#");
  posX          = getFloat(line, "$POSX#");
  posY          = getFloat(line, "$POSY#");
  cap_CG        = getFloat(line, "$CAPTEUR_CG#");
  cap_AG        = getFloat(line, "$CAPTEUR_AG#");
  cap_AD        = getFloat(line, "$CAPTEUR_AD#");
  cap_CD        = getFloat(line, "$CAPTEUR_CD#");
  etatGyro      = getInt(line, "$ETATGYRO#");
  dist          = getFloat(line, "$DIST#");
  dureeMission  = getFloat(line, "$DUREEMISSION#");
  dureeTotal    = getFloat(line, "$DUREETOTAL#");
  lastLog       = line;
}

// À appeler en loop() pour reconstituer les lignes “\n”
// et appeler parseLine() sans bloquer le serveur
void pollSerial2() {
  while (Serial2.available()) {
    // Permettre au serveur de traiter une éventuelle requête en urgence
    server.handleClient();

    char c = Serial2.read();
    if (c == '\n') {
      parseLine(serialBuf);
      serialBuf = "";
    } else {
      serialBuf += c;
      // coupe de sécurité
      if (serialBuf.length() > 300)
        serialBuf = serialBuf.substring(serialBuf.length() - 300);
    }
  }
}

// =============================================================================
// === VOTRE PAGE HTML EN PROGMEM (inchangée, j'ai collé le ‹[...]›) =========
// =============================================================================
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
    .section {
    flex: 1;
    min-width: 0;     
    padding: 10px;
    border-left: 1px solid #ddd;
    display: flex;
    flex-direction: column;
    align-items: center;
    }
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
      display: none; /* Hide the speed control section */
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
    .state .grid-info .line { margin: 4px 0; font-size: 16px; margin-bottom: 20px; }

    /* ===== 3. Journal & Carte ===== */
    .log { justify-content: flex-start; }
    .log h2 {
      margin-bottom: 40px; font-size: 24px; text-align: center;
    }
    .log-box {
      flex: 1;
      width: 100%;
      background: #f5f5f5;
      color: #333;
      padding: 10px;
      font-family: monospace;
      font-size: 14px;
      border: 1px solid #ccc;
      overflow-x: auto;    /* <=== scroll horizontal */
      overflow-y: auto;    /* <=== scroll vertical */
      white-space: pre;    /* <=== pas de retour à la ligne automatique */
      word-wrap: normal;   /* pour être sûr */
      word-break: normal;  /* pour être sûr */
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
          <button class="up"    onclick="cmd('A')">↑</button>
          <button class="left"  onclick="cmd('G')">←</button>
          <button class="stop"  onclick="cmd('S')">■</button>
          <button class="right" onclick="cmd('D')">→</button>
          <button class="down"  onclick="cmd('R')">↓</button>
        </div>
        <button class="gyrophare" onclick="cmd('B')">Gyrophare</button>
        <div class="speed-control">
          <!-- Speed control input and button are hidden -->
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
      <img src="test.png" alt="Robot BILLY">
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
    const demoPoints=[{x:1,y:1}];
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
// =============================================================================
// === HANDLERS HTTP EXISTANTS ================================================
// =============================================================================
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
  String c = server.arg("c");
  if (c=="A"||c=="R"||c=="G"||c=="D"||c=="S"||c=="B") {
    Serial2.println(c);
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Unknown cmd");
  }
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
  String v   = server.arg("s");
  String out = "*CV"+v;
  Serial2.println(out);
  server.send(200, "text/plain", "OK");
}

void handleEvents(){
  server.sendHeader("Cache-Control","no-cache");
  server.sendHeader("Connection","keep-alive");

  String s = String("retry: ") + actualisationSiteWeb + "\n"
           + "data: {"
           + "\"state\":"      + String(robotState) + ","
           + "\"battery\":0,"
           + "\"sensors\":{"
             "\"left\":"      + String(cap_CG) + ","
             "\"frontLeft\":" + String(cap_AG) + ","
             "\"frontRight\":"+ String(cap_AD) + ","
             "\"right\":"     + String(cap_CD) +
            "},"
           + "\"motors\":{"
             "\"left\":"      + String(vitG) + ","
             "\"right\":"     + String(vitD) +
            "},"
           + "\"distance\":"  + String(dist) + ","
           + "\"duration\":"  + String(dureeMission) + ","
           + "\"pos\":{"
             "\"x\":"         + String(posX) + ","
             "\"y\":"         + String(posY) +
            "},"
           + "\"gyro\":"     + String(etatGyro) + ","
           + "\"log\":\""    + lastLog + "\""
           + "}\n\n";
  server.send(200, "text/event-stream", s);
}

// =============================================================================
// === SETUP & LOOP ============================================================
// =============================================================================
void setup(){
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  delay(500);

  Serial.println("\n=== Démarrage BILLY ESP32 ===");
  WiFi.softAP(ssid, password);
  Serial.print("AP IP: "); Serial.println(WiFi.softAPIP());

  server.onNotFound(handleNotFound);
  server.on("/",             HTTP_GET, handleRoot);
  server.on("/toggleControl",HTTP_GET, handleToggle);
  server.on("/cmd",          HTTP_GET, handleCmd);
  server.on("/setSpeed",     HTTP_GET, handleSpeed);
  server.on("/events",       HTTP_GET, handleEvents);
  server.begin();

  Serial.println("Serveur HTTP démarré");
}

void loop(){

  server.handleClient();
  pollSerial2();
}