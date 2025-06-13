#include <Arduino.h>

const unsigned long intervalle =  5000;
unsigned long dernierEnvoi = 0;

void setup() {
  Serial4.begin(115200);   
}

void loop() {
  unsigned long temps = millis();
  if (temps - dernierEnvoi >= intervalle) {
    dernierEnvoi = temps;

    float vitesse = random(0, 2000) / 10.0;    
    float position = random(-500, 500) / 10.0; 

    // Formatage : $ correspond aux noms des variables et # correspond aux valeurs
    char buf[64];
    int len = snprintf(buf, sizeof(buf),
      "$VIT#%.1f$POS#%.1f\n", vitesse, position);

    Serial4.write(buf, len);
  }
}
