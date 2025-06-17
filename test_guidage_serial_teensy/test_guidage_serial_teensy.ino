#define PWMMOTEURDROIT       23
#define DIRECTIONMOTEURDROIT 21

#define PWMMOTEURGAUCHE      22
#define DIRECTIONMOTEURGAUCHE 20

#define SPEED       50
#define TURN_SPEED  50

#define TRIGGER     33
#define CAPTEUR_CG  15
#define CAPTEUR_CD  27
#define CAPTEUR_AG  31
#define CAPTEUR_AD  30

void initCapteurUltrason(){
  int i;
  int capteurs_ultrasons[4] = {CAPTEUR_CG, CAPTEUR_AG, CAPTEUR_AD, CAPTEUR_CD};

  for (i = 0; i < 4; i++) {
    pinMode(capteurs_ultrasons[i], INPUT);
  }

  pinMode(TRIGGER, OUTPUT);
}

float lectureCapteurUltrason(int capteur, int size) {
  unsigned long duree;
  float distances_cm[size];
  int i;
  float min;

  for (i = 0; i < size; i++){
      digitalWrite(TRIGGER, LOW);
      delayMicroseconds(2);
      digitalWrite(TRIGGER, HIGH);
      delayMicroseconds(10);
      digitalWrite(TRIGGER, LOW);

      duree = pulseIn(capteur, HIGH, 30000UL);

      distances_cm[i] = duree * 0.034f / 2.0f;
      delay(50);
  }

  for (i = 0; i < size; i++){ // On garde la detection minimale parmis x detections pour éviter les perturbations
    if (i == 0){
      min = distances_cm[i];
    }
    else if (distances_cm[i] < min){
      min = distances_cm[i];
    }
  }

  if (min < 5 || min > 30){ // Si c'est inférieure à 5 cm potentiellement une perturbation donc osef, > 30 aussi osef
    min = 0;
  }
  return min;
}

void initMoteurs(){
  pinMode(PWMMOTEURGAUCHE, OUTPUT);
  pinMode(DIRECTIONMOTEURGAUCHE, OUTPUT);
  pinMode(PWMMOTEURDROIT, OUTPUT);
  pinMode(DIRECTIONMOTEURDROIT, OUTPUT);
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

void avancer (float vitesse){
  runPidMoteurs(vitesse,vitesse);
  /*
  avancerMoteurGauche(pwm);
  avancerMoteurDroit(pwm);
  */
}

void reculer (float vitesse){
  runPidMoteurs(-vitesse,-vitesse);
  /*
  reculerMoteurGauche(pwm);
  reculerMoteurDroit(pwm);
  */
}

void tournerD (float vitesse){
  runPidMoteurs(vitesse,-vitesse);
  /*
  avancerMoteurGauche(pwm);
  reculerMoteurDroit(pwm);
  */
}

void tournerG (float vitesse){
  runPidMoteurs(-vitesse,vitesse);
  /*
  reculerMoteurGauche(pwm);
  avancerMoteurDroit(pwm);
  */
}


// -----------------------------------------------------------------------------
// PID
// -----------------------------------------------------------------------------



void runPidMoteurs ( float commandeMoteurGauche, float commandeMoteurDroit) {
    
    if(vitesseGauche<commandeMoteurGauche-marge){
      pwm_Gauche++;
    }else if(citesseGauche>commandeMoteurGauche+marge){
      pwm_Gauche--;
    }

    if(vitesseDroit<commandeMoteurDroit-marge){
      pwm_Droit++;
    }else if(citesseDroit>commandeMoteurDroit+marge){
      pwm_Droit--;
    }

    setPwmEtDirectionMoteurs((int)pwm_Droit, (int)pwm_Gauche);

}

void avancerDistance(int dist){
  while(distanceTotal<dist){
    avancer(SPEED);
  }
  stopMoteurs();
  
}



void setup() {
  Serial.begin(115200);
  Serial4.begin(115200);

  initMoteurs();
  initCapteurUltrason();
  Serial.printf("Yo !");
}

void loop() {
  if (lectureCapteurUltrason(CAPTEUR_AG, 3) != 0.0 || lectureCapteurUltrason(CAPTEUR_AD, 3) != 0.0){
    stopMoteurs();
    Serial.printf("Walla j'ai detecte un obstacle ! ");
  }

  if (Serial4.available()) {
    Serial.printf("ALLo");
    char c = Serial4.read();

    Serial.printf("Reçu '%c'\n", c);
    switch (c) {
      case 'A': avancer(SPEED);        break;
      case 'R': reculer(SPEED);        break;
      case 'G': tournerG(TURN_SPEED);  break;
      case 'D': tournerD(TURN_SPEED);  break;
      case 'S': stopMoteurs();         break;
      case 'M': avancerDistance(1000); break;
      default:                         break;
    }
  }
}
