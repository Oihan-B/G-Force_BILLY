#include <WiFi.h>
#include <WebServer.h>

#define RXD2 16
#define TXD2 17

// === VOTRE AP WI-FI ===
const char* ssid     = "BILLY_ESP32";
const char* password = "Rodolphe64!";

// Intervalle d’actualisation du site (en ms)
const unsigned long actualisationSiteWeb = 2000;

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
      flex: 1; max-width: 40%; height: auto; object-fit: contain;
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
      margin-bottom: 30px; font-size: 24px; text-align: center;
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
      margin-bottom: 20px;
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
      <img src="data:image/png;base64,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" alt="Robot BILLY">
      <div class="grid-info">
        <div class="col">
          <div class="line">Vitesse Moteur G : <span id="motor-left">–</span> m/s</div>
          <div class="line">Cap. Av-Gauche  : <span id="sensor-fl">–</span> cm</div>
          <div class="line">Cap. Gauche     : <span id="sensor-left">–</span> cm</div>
          <div class="line">Distance        : <span id="distance">–</span> m</div>
        </div>
        <div class="col">
          <div class="line">Vitesse Moteur D : <span id="motor-right">–</span> m/s</div>
          <div class="line">Cap. Av-Droit  : <span id="sensor-fr">–</span> cm</div>
          <div class="line">Cap. Droite     : <span id="sensor-right">–</span> cm</div>
          <div class="line">Durée           : <span id="duration">–</span> s</div>
        </div>
      </div>
    </div>
    <!-- SECTION 3 : Journal & Carte -->
      <div class="section log">
        <br />
        <h2>Logs Billy</h2>
        <div id="log" class="log-box"></div>
        <h2>BILLY Map</h2>
        <canvas id="map" width="300" height="300"></canvas>
      </div>
  </div>
  <script>
    // ==== 1) FONCTIONS GLOBALES POUR LE CONTRÔLE ====
    window.toggleControl = function(enabled) {
      fetch(`/toggleControl?enabled=${enabled}`)
        .then(r => { if(!r.ok) console.error("toggleControl failed", r.status); })
        .catch(console.error);
      document.querySelectorAll('.control-ui button, .control-ui input')
              .forEach(el => el.disabled = !enabled);
      document.getElementById('control-label')
              .innerText = enabled ? 'Contrôle activé' : 'Contrôle désactivé';
    };

    window.cmd = function(c) {
      fetch(`/cmd?c=${c}`)
        .then(r => { if(!r.ok) console.error("cmd failed", r.status); })
        .catch(console.error);
    };

    window.setSpeed = function() {
      const v = document.getElementById('speed').value;
      fetch(`/setSpeed?s=${encodeURIComponent(v)}`)
        .catch(console.error);
    };

    // ==== 2) LOG BOX QUI IGNORE LES LIGNES COMMENÇANT PAR "$" ====
    window.appendLog = function(txt) {
      if (!txt || txt.charAt(0) === '$') return;
      const l = document.getElementById('log');
      l.innerText += txt + "\n";
      l.scrollTop = l.scrollHeight;
    };

    // ==== 3) INITIALISATION DE LA MAP ====
    const canvas = document.getElementById('map'),
          ctx    = canvas.getContext('2d'),
          SCALE  = 25;  // 25 px = 1 m
    let traj = [];

    function initMap() {
      ctx.setTransform(1,0,0,1,0,0);
      ctx.clearRect(0,0,canvas.width,canvas.height);
      ctx.translate(canvas.width/2, canvas.height/2);
      // dessine axes
      ctx.strokeStyle = '#888';
      ctx.lineWidth = 1;
      ctx.beginPath();
        ctx.moveTo(0, -canvas.height/2);
        ctx.lineTo(0,  canvas.height/2);
        ctx.moveTo(-canvas.width/2, 0);
        ctx.lineTo( canvas.width/2, 0);
      ctx.stroke();
      // origine
      ctx.fillStyle = 'red';
      ctx.beginPath();
        ctx.arc(0,0,4,0,2*Math.PI);
      ctx.fill();
      traj = [];
    }

    function addPoint(x,y) {
      const px = x * SCALE,
            py = -y * SCALE;  // y+ vers le haut
      if (traj.length) {
        const p = traj[traj.length-1];
        ctx.strokeStyle = '#e74c3c';
        ctx.lineWidth = 2;
        ctx.beginPath();
          ctx.moveTo(p.px,p.py);
          ctx.lineTo(px,py);
        ctx.stroke();
      }
      ctx.fillStyle = '#3498db';
      ctx.beginPath();
        ctx.arc(px,py,3,0,2*Math.PI);
      ctx.fill();
      traj.push({px,py});
    }

    // ==== 4) POLLING JSON À /telemetry toutes les 2 s ====
    let prevLog = "";

    function refreshTelemetry(){
      fetch('/telemetry')
        .then(r => r.json())
        .then(d => {
          // -- Monitoring --
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

          // -- Log (seulement si différent ET ne commence pas par '$') --
          if (d.log && d.log !== prevLog && d.log.charAt(0) !== '$') {
            appendLog(d.log);
          }
          prevLog = d.log;

          // -- Map --
          addPoint(d.pos.x, d.pos.y);
        })
        .catch(console.error);
    }

    // ==== 5) DÉMARRAGE AU LOAD ====
    window.addEventListener('load', () => {
      toggleControl(false);
      initMap();
      refreshTelemetry();              // 1ère fois tout de suite
      setInterval(refreshTelemetry, 2000); // puis toutes les 2 s
    });
  </script>

</body>
</html>
)rawliteral";

// =============================================================================
// === HANDLERS HTTP EXISTANTS ================================================
// =============================================================================

void handleEvents(){
  server.sendHeader("Cache-Control","no-cache");
  server.sendHeader("Connection","keep-alive");

  String s = String("retry: ") + actualisationSiteWeb + "\n"
           + "data: {"
           + /* … tout votre JSON… */
           + "\"log\":\""    + lastLog + "\""
           + "}\n\n";
  server.send(200, "text/event-stream", s);
}

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

void handleTelemetry(){
  // On renvoie simplement un JSON au client
  String s = String("{")
    + "\"state\":"      + robotState
    + ",\"battery\":0"
    + ",\"sensors\":{"
      "\"left\":"      + String(cap_CG)
      + ",\"frontLeft\":" + String(cap_AG)
      + ",\"frontRight\":" + String(cap_AD)
      + ",\"right\":"     + String(cap_CD)
    + "}"
    + ",\"motors\":{"
      "\"left\":"      + String(vitG)
      + ",\"right\":"     + String(vitD)
    + "}"
    + ",\"distance\":"  + String(dist)
    + ",\"duration\":"  + String(dureeMission)
    + ",\"pos\":{"
      "\"x\":"         + String(posX)
      + ",\"y\":"         + String(posY)
    + "}"
    + ",\"gyro\":"     + String(etatGyro)
    + ",\"log\":\""    + lastLog + "\""
    + "}";
  server.send(200, "application/json", s);
}

// =============================================================================
// === SETUP & LOOP ============================================================
// =============================================================================

void setup(){
  Serial.begin(115200);
  Serial2.setRxBufferSize(512);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  serialBuf.reserve(256);
  lastLog.reserve(128);
  delay(500);

  Serial.println("\n=== Démarrage BILLY ESP32 ===");
  WiFi.softAP(ssid, password);
  Serial.print("AP IP: "); Serial.println(WiFi.softAPIP());

  server.onNotFound(handleNotFound);
  server.on("/",             HTTP_GET, handleRoot);
  server.on("/toggleControl",HTTP_GET, handleToggle);
  server.on("/cmd",          HTTP_GET, handleCmd);
  server.on("/setSpeed",     HTTP_GET, handleSpeed);
  // la seule route JSON dont on a besoin :
  server.on("/telemetry",    HTTP_GET, handleTelemetry);

  server.begin();
  Serial.println("Serveur HTTP démarré");
}

void loop(){
  pollSerial2();
  server.handleClient();
}
