#include <stdint.h>
#include <stdbool.h>

#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "ESP32-Serveur";
const char* password = "12345678";

#define RXD2 16
#define TXD2 17

WebServer server(80);
String donneesSerie = "";

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  WiFi.softAP(ssid, password);
  server.on("/", handleRoot);
  server.begin();
}

void loop() {
  while (Serial2.available() > 0) {
    char c = Serial2.read();
    donneesSerie += c;
    if (donneesSerie.length() > 2000) {
      donneesSerie = donneesSerie.substring(donneesSerie.length()-1000);
    }
  }
  server.handleClient();
}

void handleRoot() {
  String page = "<html><head><meta charset='utf-8'><title>ESP32 Serie</title></head><body>";
  page += "<h2>Serie&nbsp;:</h2><pre style='background:#eee; border:1px solid #ccc; padding:10px;'>";
  page += donneesSerie;
  page += "</pre>";
  page += "<meta http-equiv='refresh' content='1'>";
  page += "</body></html>";
  server.send(200, "text/html", page);
}

