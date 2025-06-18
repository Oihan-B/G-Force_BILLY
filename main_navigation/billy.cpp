#include <PID_v1.h>
#include "billy.h"
#include "pins.h"

// -----------------------------------------------------------------------------
// Variables Odometrie
// -----------------------------------------------------------------------------

#define SPEED 0.3;

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

double consigneDroit = 0;
double consigneGauche = 0;

// -----------------------------------------------------------------------------
// Detection Obstacles
// -----------------------------------------------------------------------------

void initCapteurUltrason(){
  int i;
  int capteurs_ultrasons[4] = {CAPTEUR_CG, CAPTEUR_AG, CAPTEUR_AD, CAPTEUR_CD};

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

void contournerObstacle() {
  arreter(); // Arrêter les moteurs pour éviter les collisions
  if (lectureCapteurUltrason(CAPTEUR_CG) != 0) {
    if (lectureCapteurUltrason(CAPTEUR_CD) !=0) {
      // Si l'obstacle est détecté à gauche et à droite, signaler avec le gyrophare
      gyro(1); // Signalisation du blocage
    }
    tournerAngleD(90); // Tourner à droite pour éviter l'obstacle
    while (lectureCapteurUltrason(CAPTEUR_CG) != 0) {
      avancer(SPEED); // Avancer pour s'éloigner de l'obstacle
    }
    avancer(SPEED); // Avancer pour reprendre la trajectoire
    tournerAngleG(90); // Revenir à la trajectoire initiale
    while (lectureCapteurUltrason(CAPTEUR_CG) != 0) {
      avancer(SPEED); // Avancer pour s'éloigner de l'obstacle
    }
    tournerAngleG(90); // Tourner à gauche pour reprendre la trajectoire
    while (lectureCapteurLigne() != 0) {
      avancer(SPEED); // Avancer pour s'éloigner de l'obstacle
    }
    tournerAngleD(90); // Revenir à la trajectoire initiale
  }
  if (lectureCapteurUltrason(CAPTEUR_CD) != 0) {
    if (lectureCapteurUltrason(CAPTEUR_CG) !=0) {
      // Si l'obstacle est détecté à gauche et à droite, signaler avec le gyrophare
      gyro(1); // Signalisation du blocage
    }
    tournerAngleG(90); // Tourner à droite pour éviter l'obstacle
    while (lectureCapteurUltrason(CAPTEUR_CD) != 0) {
      avancer(SPEED); // Avancer pour s'éloigner de l'obstacle
    }
    avancer(SPEED); // Avancer pour reprendre la trajectoire
    tournerAngleD(90); // Revenir à la trajectoire initiale
    while (lectureCapteurUltrason(CAPTEUR_CD) != 0) {
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

void initMoteurs() {
  pinMode(PWMMOTEURDROIT, OUTPUT);
  pinMode(PWMMOTEURGAUCHE, OUTPUT);
  pinMode(DIRECTIONMOTEURDROIT, OUTPUT);
  pinMode(PWMMOTEURGAUCHE, OUTPUT);
  pinMode(DIRECTIONMOTEURGAUCHE, OUTPUT);
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


void setPwmEtDirectionMoteurs(int16_t pwmGauche, int16_t pwmDroit) {
  if (pwmDroit > 0)        
    avancerMoteurDroit(pwmDroit);
  else if (pwmDroit < 0)   
    reculerMoteurDroit(-pwmDroit);

  if (pwmGauche > 0)        
    avancerMoteurGauche(pwmGauche);
  else if (pwmGauche < 0)   
    reculerMoteurGauche(-pwmGauche);

  if (pwmDroit == 0 && pwmGauche == 0) 
    stopMoteurs();
}

void avancer(float v) {
  consigneDroit = consigneGauche = v;
}

void reculer(float v) {
  consigneDroit = consigneGauche = -v;
}

void tournerD(float v) {
  consigneGauche =  v;
  consigneDroit  = -v;
}

void tournerG(float v) {
  consigneGauche = -v;
  consigneDroit  =  v;
}

void tournerDsoft(float v, float percent) {
  consigneGauche = v;
  consigneDroit  = percent * v;
}

void tournerGsoft(float v, float percent) {
  consigneGauche = percent * v;
  consigneDroit  = v;
}

void arreter(){
  consigneGauche = consigneDroit = 0;
}

void tournerAngleD (uint8_t angle) {
  while (angleTotal<angle-0.1){
    tournerD(SPEED);
  }
  arreter();
}

void tournerAngleG (uint8_t angle) {
  while (angleTotal>angle+0.1){
    tournerG(SPEED);
  }
  arreter();
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
    /*
    if(angleTotal>pi){
      angleTotal-=2*pi;
    }else if(angleTotal<-pi){
      angleTotal+=2*pi;
    }*/
    x+=distMoy*cos(angleTotal);
    y+=distMoy*sin(angleTotal);
    compteDroit=0;
    compteGauche=0;

    runPidMoteurs(consigneGauche, consigneDroit);
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


int distanceAtteinte(int dist){
  if(dist>=distanceTotale){
    return 1;
  }
  return 0;
}

// -----------------------------------------------------------------------------
// PID
// -----------------------------------------------------------------------------


void runPidMoteurs(float cmdG, float cmdD) {
  // inchangé
  if (vitesseGauche < cmdG - marge)  pwm_Gauche++;
  else if (vitesseGauche > cmdG + marge) pwm_Gauche--;

  if (vitesseDroit < cmdD - marge)   pwm_Droit++;
  else if (vitesseDroit > cmdD + marge)   pwm_Droit--;

  // bornage
  pwm_Gauche = constrain(pwm_Gauche, -255, 255);
  pwm_Droit  = constrain(pwm_Droit,  -255, 255);

  setPwmEtDirectionMoteurs((int)pwm_Gauche, (int)pwm_Droit);
}

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


// -----------------------------------------------------------------------------
// IHM
// -----------------------------------------------------------------------------

int bouton_presse(){
  for(int i = 0; i < NB_BOUTONS; i++){
    int pin = boutons[i];
    if(digitalRead(pin) == LOW){
      delay(20);
      while(digitalRead(pin) == LOW);
      delay(20);
      return pin;
    }
  }
  return -1;
}

void afficherEcran(*char txt1, *char txt2, *char txt3, *char txt4){
  if(txt1){
    lcd.setCursor(0, 0);
    lcd.print(txt1);
  }
  if(txt2){
    lcd.setCursor(0, 1);
    lcd.print(txt2);
  }
  if(txt3){
    lcd.setCursor(0, 2);
    lcd.print(txt3);
  }
  if(txt4){
    lcd.setCursor(0, 3);
    lcd.print(txt4);
  }
}
