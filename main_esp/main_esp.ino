#include <WiFi.h>
#include <WebServer.h>

#define RXD2 16
#define TXD2 17

// === VOTRE AP WI-FI ===
const char* ssid     = "BILLY_ESP32";
const char* password = "Rodolphe64!";

// Intervalle d’actualisation du site (en ms)
const unsigned long actualisationSiteWeb = 1500;

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
      <img src="data:image/png;base64,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
      FOV+Ss6hi9z1OdKZfc0TOUOzKW006t6+z/x4nOrevsx+dm5icdMpJp5x0Gp10ykmnnHTKSafRSaecdMpJp5x0Gp10ykmnnHTKSafRSaecdMpJp5x0Gp10ykmnnHTKSafRSaecdMpJp9Fp
      dNIpJ51y0ml00iknnXLSKSedRiedctIpJ51y0ml00iknnXLS+fKc39cE1tIpJ52Pc8Z1R+e8OS+rrqVTTjrplJNOOemUk0456aRTTjrlpFNOOuWkk0456ZSTTjnplJNOOuWkU0465aRTT
      jrplJPOJHPGdtWDTjc10OkelffP+VXX59WPmk6j093XctIpJ51y0ml00iknnXLSKSedRiedctIpJ51y0ml00iknnXLSKSedRiedctIpJ51Gp9FJp5x0ykmn0UmnnHTKSaecdBqddMpJp5
      x0ykmn0UmnnHTKSaecdBqddMpJp5x0ykmn0UmnnHTKSaecdBqddMpJp5x0CkKn0UmnnHTKSafRaXTSKSedctJpdNIpJ51y0iknnUYnnXLSKSedctJpdNIpJ51y0iknnUYnnXLSKSedctJ
      pdNIpJ51y0iknnUYnnXLSKSedRqfRSaecdMpJp9Gpp5x0ykmnnHQanXTKSaecdMpJp9FJp5x0ykmnnHQanXTKSaecdMpJp9FJp5x0ykmnnHQanXTKSaecdMpJp9FJp5x0ykmn0Wl00ikn
      nXLSaXTSKSedctIpJ51GJ51y0iknnXLSaXTSKSedctIpJ51GJ51y0iknnXLSaXTSKSedctIpJ51GJ51y0iknnUan0UmnnHTKSafRqaacdMpJp5x0Gp10ykmnnHTKSafRSaecdMpJp5x0G
      p10ykmnnHTKSafRSaecdMpJp5x0Gp10ykmnnHTKSafRSaecdMpJp9FpdNIpJ51y0ml00iknnXLSKSedRiedctIpJ51y0ml00iknnXLSKSedthGduZx0mnMnnXLSKSedRqfRSafrTjrpdO
      40OvWUk0456ZSTTnPdSSeddMpJp5x0Gp10ykmnnHS67qTTnDvplJNOOemUk06jk0456ZSTTjnpNDrplJNOOemUk06jk0456ZSTTqPT6KRTTjrlpNPopFNOOuWkU046jU465aRTTjrlpNP
      opFNOOuWkU046jU465aRTTjrlpNPopFNOOuWkU046jU465aRTTjqNTqOTTjnplJNOo1NNOemUk0456TQ66ZSTTjnplJNOo5PONZfLOX1y0unJlk466TQ65Uwl58kmLa2cl8amLSaU02ZY
      VWQpLJQHm77jZxo6s5B3C/ej+8/+XXh4/C0zMzMzMzMzMzMzMzMzsyf2C9zGtVFvFjRMAAAAAElFTkSuQmCC" alt="Robot BILLY">
      <div class="grid-info">
        <div class="col">
          <div class="line">Vitesse Moteur G: <span id="motor-left">–</span> m/s</div>
          <div class="line">Cap. Av-Gauche  : <span id="sensor-fl">–</span> cm</div>
          <div class="line">Cap. Gauche     : <span id="sensor-left">–</span> cm</div>
          <div class="line">Distance        : <span id="distance">–</span> m</div>
        </div>
        <div class="col">
          <div class="line">Vitesse Moteur D: <span id="motor-right">–</span> m/s</div>
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

    window.addEventListener('load', () => {
      // ==== 3) INITIALISATION DE LA MAP ====
      const canvas = document.getElementById('map'),
            ctx    = canvas.getContext('2d'),
            SCALE  = 25;            // 25 px = 1 m
      let traj = [];

      function initMap() {
        ctx.setTransform(1,0,0,1,0,0);
        ctx.clearRect(0,0,canvas.width,canvas.height);
        // origine au centre
        ctx.translate(canvas.width/2, canvas.height/2);
        // axes
        ctx.strokeStyle = '#888';
        ctx.lineWidth = 1;
        ctx.beginPath();
          ctx.moveTo(0, -canvas.height/2);
          ctx.lineTo(0,  canvas.height/2);
          ctx.moveTo(-canvas.width/2, 0);
          ctx.lineTo( canvas.width/2, 0);
        ctx.stroke();
        // point origine
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

      // ==== 4) DÉMARRAGE ====
      toggleControl(false);
      initMap();

      // ==== 5) OUVERTURE DU SSE / EventSource ====
      if (!window.EventSource) {
        console.error("EventSource not supported");
        return;
      }
      const es = new EventSource('/events');
      es.onmessage = e => {
        const d = JSON.parse(e.data);

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

        // -- Log & Map --
        appendLog(d.log);
        addPoint(d.pos.x, d.pos.y);
      };

      es.onerror = err => {
        console.error("SSE error", err);
        es.close();
      };
    });
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