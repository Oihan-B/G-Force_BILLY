#define PWMMOTEURDROIT 23
#define DIRECTIONMOTEURDROIT 21

#define PWMMOTEURGAUCHE 22
#define DIRECTIONMOTEURGAUCHE 20

#define SPEED 100
#define TURN_SPEED 100

#define S1 2
#define S2 3
#define S3 4
#define S4 5
#define S5 6
#define CLP 9

#define TRIGGER 33
#define CAPTEUR_CG 15
#define CAPTEUR_CD 27
#define CAPTEUR_AG 31
#define CAPTEUR_AD 30

int capteurs[5] = {S1, S2, S3, S4, S5};
int capteurs_ultrasons[4] = {CAPTEUR_CG, CAPTEUR_AG, CAPTEUR_AD, CAPTEUR_CD};
int boutons[4] = {0, 1, 7, 8};

void setup() {
  Serial.begin(9600);
  pinMode(DIRECTIONMOTEURDROIT, OUTPUT);
  pinMode(DIRECTIONMOTEURGAUCHE, OUTPUT);

  int i;

  for (i = 0; i < 5; i++) {
    pinMode(capteurs[i], INPUT_PULLUP);
  }
  for (i = 0; i < 4; i++) {
    pinMode(boutons[i], INPUT_PULLUP);
  }
  for (i = 0; i < 4; i++) {
    pinMode(capteurs_ultrasons[i], INPUT_PULLUP);
  }
  pinMode(TRIGGER, OUTPUT);
}

float lectureCapteurUltrason(int capteur) {
  unsigned long duree;
  float distance_cm;

  digitalWrite(TRIGGER, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER, LOW);

  duree = pulseIn(capteur, HIGH, 30000UL);

  distance_cm = duree * 0.034f / 2.0f;

  if (distance_cm < 0 || distance_cm > 50){
    distance_cm = 0;
  }

  return distance_cm;
}


void avancerMoteurDroit(uint8_t pwm) {
  analogWrite (PWMMOTEURDROIT, pwm); // Contrôle de vitesse en PWM
  digitalWrite(DIRECTIONMOTEURDROIT, LOW);
}

void avancerMoteurGauche(uint8_t pwm) {
  analogWrite (PWMMOTEURGAUCHE, pwm); // Contrôle de vitesse en PWM
  digitalWrite(DIRECTIONMOTEURGAUCHE, LOW);
}

void reculerMoteurDroit (uint8_t pwm) {
  analogWrite (PWMMOTEURDROIT, pwm); // Contrôle de vitesse en PWM
  digitalWrite(DIRECTIONMOTEURDROIT, HIGH);
}

void reculerMoteurGauche (uint8_t pwm) {
  analogWrite (PWMMOTEURGAUCHE, pwm); // Contrôle de vitesse en PWM
  digitalWrite(DIRECTIONMOTEURGAUCHE, HIGH);
}

void stopMoteurs() {
  analogWrite (PWMMOTEURDROIT, 0);
  digitalWrite(DIRECTIONMOTEURDROIT, LOW);
  analogWrite (PWMMOTEURGAUCHE, 0);
  digitalWrite(DIRECTIONMOTEURGAUCHE, LOW);
}

void avancer (int16_t pwm){
  avancerMoteurGauche(pwm);
  avancerMoteurDroit(pwm);
}

void reculer (int16_t pwm){
  reculerMoteurGauche(pwm);
  reculerMoteurDroit(pwm);
}

void tournerD (int16_t pwm){
  avancerMoteurGauche(pwm);
  reculerMoteurDroit(pwm);
}

void tournerG (int16_t pwm){
  reculerMoteurGauche(pwm);
  avancerMoteurDroit(pwm);
}

int bouton_presse(){
  int trouve=0, ret=0;

  for(int i=0;i < 4 ; i++){
    if(digitalRead(boutons[i])==LOW){
      if(trouve) return 0;
      ret = i+1; trouve=1;
    }
  }
  return ret;
}

void loop (){

  int detections[5] = {0, 0, 0, 0, 0};
  int i;

  while (1){


    // TEST BOUTONS
    /*
    int bouton = bouton_presse();
    Serial.println("\n");
    Serial.println(bouton);
    Serial.println("\n");
    */
    // TEST CAPTEURS ULTRASON
    /*
    float val_ultrasons[4];

    for (i = 0; i < 4; i++){
      val_ultrasons[i] = lectureCapteurUltrason(capteurs_ultrasons[i]); 
      delay(100);
    }

    Serial.println("\n");
    for (i = 0; i < 4; i++){
      Serial.println(val_ultrasons[i]);
    }
    Serial.println("\n");
    */
    // TEST SUIVI DE LIGNES

    int c_avancer = 0, c_tournerD = 0, c_tournerG = 0;

    delay(500);

    for (i = 0; i < 5; i++){
      detections[i] = digitalRead(capteurs[i]); // 0 SOL // 1 LIGNE
    }

    detections[0] = 0; // Il marche pas 

    for (i = 0; i < 5; i++){
      Serial.println(detections[i]);
    }

    if (detections[1] && detections[2] && detections[3]) {
      Serial.println("\nCheckpoint detecte\n");
      stopMoteurs();
    }

    else if (detections[2]) {
      Serial.println("\nAvancer tout droit\n");
      c_avancer += 1;

      if (c_avancer > 1){
        avancer(SPEED);
        c_avancer = 0;
      }
    } 

    else if (detections[3] || detections[4]) {
      Serial.println("\nTourner à droite pour retrouver la ligne\n");
      c_tournerD += 1;

      if (c_tournerD > 1){
        tournerD(TURN_SPEED);
        c_tournerD = 0;
      }
    } 

    else if (detections[0] || detections[1]) {
      Serial.println("\nTourner à gauche pour retrouver la ligne\n");
      c_tournerG += 1;

      if (c_tournerG > 1){
        tournerG(TURN_SPEED);
        c_tournerG = 0;
      }
    } 

    else {
      Serial.println("\nLigne perdue : STOP ou reculer\n");
      stopMoteurs();
    }
  }
}