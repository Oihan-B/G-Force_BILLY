#include <AFMotor.h>

const int joystickPinX = A0;  // Pin pour l'axe X
const int joystickPinY = A1;  // Pin pour l'axe Y

#define PWMMOTEURDROIT 6
#define DIRECTIONMOTEURDROIT 7
#define PWMMOTEURGAUCHE 5
#define DIRECTIONMOTEURGAUCHE 4
#define SPEED 180

#define S1 8
#define S2 9
#define S3 10
#define S4 11
#define S5 12
#define CLP 13

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
  analogWrite (PWMMOTEURDROIT, pwm); // Contrôle de vitesse en PW
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

int lectureCapteurLigne(int capteur_id) {
  return analogRead(capteur_id);
}

void calibrationSuiviLignes(int seuils[5]) {
  int mesures[5][100]; //100 mesures pour chaque capteur
	int i, c;
	
  for (i = 0; i < 100; i++) {
    for (c = 0; c < 5; c++) {
        mesures[c][i] = lectureCapteurLigne(c);
    }
  }

  for (c = 0; c < 5; c++) {
    int min = mesures[c][0];
    int max = mesures[c][0];
    for (i = 1; i < 100; i++) {
      if (mesures[c][i] < min)
          min = mesures[c][i];
      if (mesures[c][i] > max)
          max = mesures[c][i];
    }
    seuils[c] = (max + min) / 2;
  }
}

void loop (){

  int capteurs[5] = {S1, S2, S3, S4, S5};
  int seuils[5];
  int detections[5];
  int i;
  calibrationSuiviLignes(seuils);

  while (1){

    for (i = 0; i < 5; i++){
      if (lectureCapteurLigne(capteurs[i]) > seuils[i]){
        detections[i] = 0; //SOL
      }
        else{
      detections[i] = 1; //LIGNE
      }
    }

    for (i = 0; i < 5; i++){
      Serial.println(detections[i]);
    }

    delay(100);

    if (detections[2]) {
      Serial.println("\nAvancer tout droit\n");
    } 
    else if (detections[3] || detections[4]) {
      Serial.println("\nTourner à droite pour retrouver la ligne\n");
    } 
    else if (detections[0] || detections[1]) {
      Serial.println("\nTourner à gauche pour retrouver la ligne\n");
    } 
    else if (detections[1] && detections[2] && detections[3]) {
      Serial.println("\nTourner à gauche pour retrouver la ligne\n");
    } 
    else {
      Serial.println("\nLigne perdue : STOP ou reculer\n");
    }

  }

}

