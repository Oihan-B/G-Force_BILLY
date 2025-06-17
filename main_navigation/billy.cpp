#include <stdint.h>
#include <stdbool.h>
#include "billy.h"
#include "pins.h"

// -----------------------------------------------------------------------------
// Variables Odometrie
// -----------------------------------------------------------------------------

#define SPEED 100;

#define ENTRAXE 320 
#define NB_TIC 1560.0 // Nombre de tic par tour de roue
#define D_ROUE 100 // Diametre roue
#define TIMERINTERVALE 20000 // ms

IntervalTimer myTimer;

volatile double pi=math.pi;
volatile int16_t compteDroit = 0;  // comptage de tics d'encoder qui sera incrémenté sur interruption " On change " sur l'interruption 0 
volatile int16_t compteGauche = 0; // comptage de tics d'encoder qui sera incrémenté sur interruption " On change " sur l'interruption 1 
volatile double dist=0;
volatile double distMoy=0;
volatile double distDroit=0;
volatile double distGauche=0;
volatile double vitesseDroit = 0;  // vitesse du moteur en tics
volatile double vitesseGauche = 0; // vitesse du moteur en tics
volatile double pwm_Droit=60;
volatile double pwm_Gauche=60;
volatile double intervalle=0.2;
volatile double distanceTotal = 0; //mm
volatile double angleTotal = 0; // radian
volatile double x = 0; //mm 
volatile double y = 0; //mm
volatile double theta = 0; // radian entre -Pi et Pi


// -----------------------------------------------------------------------------
// Detection Obstacles
// -----------------------------------------------------------------------------


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

void contournerObstacle() {
  stopMoteurs(); // Arrêter les moteurs pour éviter les collisions
  if (lectureCapteurUltrason(CAPTEUR_CG, 2) != 0) {
    if (lectureCapteurUltrason(CAPTEUR_CD, 2) !=0) {
      // Si l'obstacle est détecté à gauche et à droite, signaler avec le gyrophare
      gyro(1); // Signalisation du blocage
    }
    tournerAngleD(90); // Tourner à droite pour éviter l'obstacle
    while (lectureCapteurUltrason(CAPTEUR_CG, 2) != 0) {
      avancer(SPEED); // Avancer pour s'éloigner de l'obstacle
    }
    avancer(SPEED); // Avancer pour reprendre la trajectoire
    tournerAngleG(90); // Revenir à la trajectoire initiale
    while (lectureCapteurUltrason(CAPTEUR_CG, 2) != 0) {
      avancer(SPEED); // Avancer pour s'éloigner de l'obstacle
    }
    tournerAngleG(90); // Tourner à gauche pour reprendre la trajectoire
    while (lectureCapteurLigne() != 0) {
      avancer(SPEED); // Avancer pour s'éloigner de l'obstacle
    }
    tournerAngleD(90); // Revenir à la trajectoire initiale
  }
  if (lectureCapteurUltrason(CAPTEUR_CD, 2) != 0) {
    if (lectureCapteurUltrason(CAPTEUR_CG, 2) !=0) {
      // Si l'obstacle est détecté à gauche et à droite, signaler avec le gyrophare
      gyro(1); // Signalisation du blocage
    }
    tournerAngleG(90); // Tourner à droite pour éviter l'obstacle
    while (lectureCapteurUltrason(CAPTEUR_CD, 2) != 0) {
      avancer(SPEED); // Avancer pour s'éloigner de l'obstacle
    }
    avancer(SPEED); // Avancer pour reprendre la trajectoire
    tournerAngleD(90); // Revenir à la trajectoire initiale
    while (lectureCapteurUltrason(CAPTEUR_CD, 2) != 0) {
      avancer(SPEED); // Avancer pour s'éloigner de l'obstacle
    }
    tournerAngleD(90); // Tourner à gauche pour reprendre la trajectoire
    while (lectureCapteurLigne() != 0) {
      avancer(SPEED); // Avancer pour s'éloigner de l'obstacle
    }
    tournerAngleG(90); // Revenir à la trajectoire initiale
  }
}


// -----------------------------------------------------------------------------
// Suivi Lignes
// -----------------------------------------------------------------------------


void initSuiviLigne(){
  int capteurs[5] = {S1, S2, S3, S4, S5};
  int i;

  for (i = 0; i < 5; i++) {
    pinMode(capteurs[i], INPUT_PULLUP);
  }
}

int lectureCapteurLigne(int capteur_id) {
    return digitalRead(capteur_id) ;
}

char suiviLigne(){ 

  int detections[5];

  for (i = 0; i < 5; i++){
    detections[i] = lectureCapteurLigne(i)); // 0 SOL // 1 LIGNE
  }

  if (detections[1] && detections[2] && detections[3]) {
    return "C";
  }

  else if (detections[2]) {
    return "A";
  } 

  else if (detections[3] || detections[4]) {
    return "D";
  } 

  else if (detections[0] || detections[1]) {
    return "G";
  } 

  else {
    return "S";
  }
}
// A (avancer en suivant la ligne)
// D (se décaler vers la droite pour récupérer la ligne)
// G (se décaler vers la gauche pour récupérer la ligne)
// S (s'arrêter car ligne perdue)
// C (s'arrêter car checkpoint detectée)


// -----------------------------------------------------------------------------
// Commandes Moteurs
// -----------------------------------------------------------------------------

void initMoteurs(){
  pinMode(PWMMOTEURGAUCHE,      OUTPUT);
  pinMode(DIRECTIONMOTEURGAUCHE,OUTPUT);
  pinMode(PWMMOTEURDROIT,       OUTPUT);
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

void avancer (uint8_t pwm){
  avancerMoteurGauche(pwm);
  avancerMoteurDroit(pwm);
}

void reculer (uint8_t pwm){
  reculerMoteurGauche(pwm);
  reculerMoteurDroit(pwm);
}

void tournerD (uint8_t pwm){
  avancerMoteurGauche(pwm);
  reculerMoteurDroit(pwm);
}

void tournerDsoft (uint8_t pwm, uint8_t pp){
  avancerMoteurGauche(pwm);
  avancerMoteurDroit((pp / 100) * pwm);
}

void tournerG (uint8_t pwm){
  reculerMoteurGauche(pwm);
  avancerMoteurDroit(pwm);
}

void tournerGsoft (uint8_t pwm, uint8_t pp){
  avancerMoteurDroit(pwm);
  avancerMoteurGauche((pp / 100) * pwm);
}


void tournerAngleD (uint8_t angle) {
  while (angleTotal<angle-0.1){
    tournerD(SPEED);
  }
  stopMoteurs();
}

void tournerAngleG (uint8_t angle) {
  while (angleTotal>angle+0.1){
    tournerG(SPEED);
  }
  stopMoteurs();
}

// -----------------------------------------------------------------------------
// Fonctions Odometrie
// -----------------------------------------------------------------------------

void initEncodeurs() {
  myTimer.begin(interruptionTimer, TIMERINTERVALE);
  pinMode(ENCODEURDROITA, INPUT);
  pinMode(ENCODEURDROITB, INPUT);
  pinMode(ENCODEURGAUCHEA, INPUT);
  pinMode(ENCODEURGAUCHEB, INPUT);
  attachInterrupt(digitalPinToInterrupt(ENCODEURDROITA), compterDroit, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODEURGAUCHEA), compterGauche, CHANGE);
}

void interruptionTimer(){
  
    //Calcul des vitesses, position et angle du robot  
    vitesseDroit=((compteDroit*50)/(NB_TIC));
    vitesseGauche=((compteGauche*50)/(NB_TIC));
    
    distMoy=(distDroit+distGauche)/2;
    distanceTotal+=distMoy;
    if(distDroit>distGauche || distGauche>distDroit){
      dist=distDroit-distGauche;
      theta = dist/ENTRAXE;
    }
    angleTotal+=theta;
    if(angleTotal>pi){
      angleTotal-=2*pi;
    }else if(angleTotal<-pi){
      angleTotal+=2*pi;
    }
    x+=distMoy*cos(angleTotal);
    y+=distMoy*sin(angleTotal);
    compteDroit=0;
    compteGauche=0;
}


void compterDroit() {
  
  if(digitalRead(ENCODEURDROITA) == digitalRead(ENCODEURDROITB)){
    compteDroit++;
  }else {
    compteDroit--;
  }
  distDroit=((pi*D_ROUE)/NB_TIC)*compteDroit;
}

void compterGauche() {
  if(digitalRead(ENCODEURGAUCHEA) == digitalRead(ENCODEURGAUCHEB)){
    compteGauche--;
  }else{
    compteGauche++;
  }
  distGauche=((pi*D_ROUE)/NB_TIC)*compteGauche;
}


// -----------------------------------------------------------------------------
// Monitoring ESP32
// -----------------------------------------------------------------------------

void monitoring (){
  
}
/*collecte de toutes les données relatives au robot et à la mission et actualisation du site web de supervision*/

// -----------------------------------------------------------------------------
// GYRO
// -----------------------------------------------------------------------------

void gyro (uint8_t etat){
  if (etat == 1){
    digitalWrite(GYROPHARE, HIGH);
  }
  else {
    digitalWrite(GYROPHARE, HIGH);
  }
}
