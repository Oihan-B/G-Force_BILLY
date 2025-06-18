#include "billy.h"
#include "pins.h"

float pwm_Gauche = 0, pwm_Droit = 0;
float vitesseGauche = 0, vitesseDroit = 0; // à mettre à jour depuis vos codeurs
float distanceTotal   = 0;                  // idem
const float marge     = 0.05;

unsigned long lastSupervision = 0;


void actualiser_supervision(float vit, float);


void setup() {
  Serial.begin(115200); 
  Serial4.begin(115200);
  
  initMoteurs();
  initCapteurUltrason();
}

void loop() {
  unsigned long now = millis();

  //--- 1) Obstacle avant tout
  float dAG = lectureCapteurUltrason(CAPTEUR_AG, 3);
  float dAD = lectureCapteurUltrason(CAPTEUR_AD, 3);
  if ( (dAG>5 && dAG<30) || (dAD>5 && dAD<30) ) {
    stopMoteurs();
    Serial.println("Obstacle détecté !");
  }

  //--- 2) Lecture des ordres venant de l'ESP32
  while (Serial4.available()) {
    char c = Serial4.read();
    switch (c) {
      case 'A': avancer(SPEED);        break;
      case 'R': reculer(SPEED);        break;
      case 'G': tournerG(TURN_SPEED);  break;
      case 'D': tournerD(TURN_SPEED);  break;
      case 'S': stopMoteurs();         break;
      case 'M': avancerDistance(1000); break;
      default: /* ignore */            break;
    }
  }

  //--- 3) Envoi périodique de la supervision
  if (now - lastSupervision >= SUPER_INTERVAL) {
    lastSupervision = now;
    // Récupérez ici vos vraies variables :
    float vitesse = (vitesseGauche + vitesseDroit) * 0.5f;
    float posX     = distanceTotal;      // exemple
    float posY     = 0.0f;               // si vous avez Y
    actualiser_supervision(vitesse, posX, posY);
  }
}

void actualiser_supervision(float vit, float posX, float posY) {
  // Exemple de trame : $VIT#12.3$POSX#45.6$POSY#0.0\n
  char buf[64];
  int len = snprintf(buf, sizeof(buf),
                     "$VIT#%.2f$POSX#%.2f$POSY#%.2f\n",
                     vit, posX, posY);
  Serial4.write(buf, len);
}
