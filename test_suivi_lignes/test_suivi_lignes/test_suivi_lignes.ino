#define PWMMOTEURDROIT 23
#define DIRECTIONMOTEURDROIT 21

#define PWMMOTEURGAUCHE 22
#define DIRECTIONMOTEURGAUCHE 20

#define SPEED 100
#define TURN_SPEED 100

#define S1 24
#define S2 25
#define S3 14
#define S4 15
#define S5 17
#define CLP 16

void setup() {
  Serial.begin(9600);
  pinMode(DIRECTIONMOTEURDROIT, OUTPUT);
  pinMode(DIRECTIONMOTEURGAUCHE, OUTPUT);

  pinMode(S1, INPUT);
  pinMode(S2, INPUT);
  pinMode(S3, INPUT);
  pinMode(S4, INPUT);
  pinMode(S5, INPUT);
  pinMode(CLP, INPUT);
}

void avancerMoteurDroit(uint8_t pwm) {
  analogWrite (PWMMOTEURDROIT, pwm); // Contrôle de vitesse en PWM
  digitalWrite(DIRECTIONMOTEURDROIT, HIGH);
}

void avancerMoteurGauche(uint8_t pwm) {
  analogWrite (PWMMOTEURGAUCHE, pwm); // Contrôle de vitesse en PWM
  digitalWrite(DIRECTIONMOTEURGAUCHE, HIGH);
}

void reculerMoteurDroit (uint8_t pwm) {
  analogWrite (PWMMOTEURDROIT, pwm); // Contrôle de vitesse en PWM
  digitalWrite(DIRECTIONMOTEURDROIT, LOW);
}

void reculerMoteurGauche (uint8_t pwm) {
  analogWrite (PWMMOTEURGAUCHE, pwm); // Contrôle de vitesse en PWM
  digitalWrite(DIRECTIONMOTEURGAUCHE, LOW);
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

int lectureCapteurLigne(int capteur_id) {
  return analogRead(capteur_id);
}

void calibrationSuiviLignes(int seuils[5]) {

  Serial.println("\nDébut phase de CALIBRATION\n");
  
  int mesures[5][100]; //100 mesures pour chaque capteur
	int i, c;
	
  for (i = 0; i < 100; i++) {
    for (c = 0; c < 5; c++) {
        mesures[c][i] = lectureCapteurLigne(c);
        delay(20);
    }
    delay(50);
  }

  for (c = 0; c < 5; c++) {
    int min = mesures[c][0];
    int max = mesures[c][0];
    for (i = 1; i < 100; i++) {
      if (mesures[c][i] < min){
        min = mesures[c][i];
      }
      if (mesures[c][i] > max){
        max = mesures[c][i];
      }
    }
    seuils[c] = (max + min) / 2;
  }

  Serial.println("\nFin phase de CALIBRATION\n");
  for (c = 0; c < 5; c++) {
    Serial.println(seuils[c]);
    Serial.println(" ");
  }
}

void loop (){

  int capteurs[5] = {S1, S2, S3, S4, S5};
  int seuils[5] = {512, 512, 512, 512, 512};
  int detections[5];
  int i;

  //calibrationSuiviLignes(seuils);

  while (1){

    int c_avancer = 0, c_tournerD = 0, c_tournerG = 0;

    for (i = 0; i < 5; i++){
      if (lectureCapteurLigne(capteurs[i]) > seuils[i]){
        detections[i] = 0; //SOL
      }
      else{
        detections[i] = 1; //LIGNE
      }
      Serial.println(detections[i]);
      Serial.println(" ");
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