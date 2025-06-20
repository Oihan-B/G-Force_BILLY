#include <PID_v1.h>
#include <math.h>
#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "billy.h"
#include "pins.h"


// -----------------------------------------------------------------------------
// Variables Odometrie
// -----------------------------------------------------------------------------

IntervalTimer myTimer;

volatile double pi=3.14159;
volatile double compteDroit = 0;  // comptage de tics d'encoder qui sera incrémenté sur interruption " On change " sur l'interruption 0 
volatile double compteGauche = 0; // comptage de tics d'encoder qui sera incrémenté sur interruption " On change " sur l'interruption 1 
volatile double dist=0;
volatile double distMoy=0;
volatile double distDroit=0;
volatile double distGauche=0;
volatile double vitesseDroit = 0;  // vitesse du moteur en tics
volatile double vitesseGauche = 0; // vitesse du moteur en tics
volatile double pwm_Droit=0;
volatile double pwm_Gauche=0;
volatile double intervalle=0.2;
volatile double distanceTotal = 0; //mm
volatile double angleTotal = 0; // radian
volatile double x = 0; //mm 
volatile double y = 0; //mm
volatile double theta = 0; // radian entre -Pi et Pi

float marge = 0.1;

double ancienConsigneDroit = 0;
double ancienConsigneGauche = 0;
double consigneDroit = 0;
double consigneGauche = 0;

float AG;
float AD;
float CG;
float CD;

double derniereMAJ;
double tempsMAJ = 3000;

int etatGyro = 0;

float dureeTotal;
float dureeMission;
float debutMission;
int etatRobot=0;

char *rxBuf[10];


void startMission(int s, float t){
  etatRobot = s;
  debutMission = t;
}

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
  float min = 0;

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

void contournerObstacle(float vit) {
  /*
  arreter(); // Arrêter les moteurs pour éviter les collisions
  if (lectureCapteurUltrason(CAPTEUR_CG, 3) != 0) {
    if (lectureCapteurUltrason(CAPTEUR_CD, 3) !=0) {
      // Si l'obstacle est détecté à gauche et à droite, signaler avec le gyrophare
      gyro(1); // Signalisation du blocage
    }
    tournerAngleD(vit, pi/2); // Tourner à droite pour éviter l'obstacle
    while (lectureCapteurUltrason(CAPTEUR_CG, 3) != 0) {
      avancer(vit); // Avancer pour s'éloigner de l'obstacle
    }
    avancer(vit); // Avancer pour reprendre la trajectoire
    tournerAngleG(vit, pi/2); // Revenir à la trajectoire initiale
    while (lectureCapteurUltrason(CAPTEUR_CG, 3) != 0) {
      avancer(vit); // Avancer pour s'éloigner de l'obstacle
    }
    tournerAngleG(vit, pi/2); // Tourner à gauche pour reprendre la trajectoire
    while (lectureCapteurLigne() != 0) {
      avancer(vit); // Avancer pour s'éloigner de l'obstacle
    }
    tournerAngleD(vit, pi/2); // Revenir à la trajectoire initiale
  }
  if (lectureCapteurUltrason(CAPTEUR_CD, 3) != 0) {
    if (lectureCapteurUltrason(CAPTEUR_CG, 3) !=0) {
      // Si l'obstacle est détecté à gauche et à droite, signaler avec le gyrophare
      gyro(1); // Signalisation du blocage
    }
    tournerAngleG(vit, pi/2); // Tourner à droite pour éviter l'obstacle
    while (lectureCapteurUltrason(CAPTEUR_CD, 3) != 0) {
      avancer(vit); // Avancer pour s'éloigner de l'obstacle
    }
    avancer(vit); // Avancer pour reprendre la trajectoire
    tournerAngleD(vit, pi/2); // Revenir à la trajectoire initiale
    while (lectureCapteurUltrason(CAPTEUR_CD, 3) != 0) {
      avancer(vit); // Avancer pour s'éloigner de l'obstacle
    }
    tournerAngleD(vit, pi/2); // Tourner à gauche pour reprendre la trajectoire
    while (lectureCapteurLigne() != 0) {
      avancer(vit); // Avancer pour s'éloigner de l'obstacle
    }
    tournerAngleG(vit, pi/2); // Revenir à la trajectoire initiale
  }
  */
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
  int capteurs[5] = {S1, S2, S3, S4, S5};
  int detections[5];
  int i;

  for (i = 0; i < 5; i++){
    detections[i] = digitalRead(capteurs[i]); // 0 SOL // 1 LIGNE
  }

  //META CAPTEUR CASSE JONATHAN

  detections[3] = 1;
  detections[4] = 1;

  if (!detections[0] && !detections[1] && !detections[2]) {
    return 'C';
  }

  else if (!detections[2]) {
    return 'D';
  } 

  else if (!detections[0]) {
    return 'G';
  } 

  else if (!detections[1]) {
    return 'A';
  }

  else {
    return 'S';
  }

  //META CAPTEUR PAS CASSE
  /*
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
  */
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
  pinMode(DIRECTIONMOTEURDROIT, OUTPUT);
  pinMode(PWMMOTEURGAUCHE, OUTPUT);
  pinMode(DIRECTIONMOTEURGAUCHE, OUTPUT);
  stopMoteurs();
}

void avancerMoteurDroit(int pwm) {
  analogWrite (PWMMOTEURDROIT, pwm); // Contrôle de vitesse en PWM
  digitalWrite(DIRECTIONMOTEURDROIT, LOW);
}

void avancerMoteurGauche(int pwm) {
  analogWrite (PWMMOTEURGAUCHE, pwm); // Contrôle de vitesse en PWM
  digitalWrite(DIRECTIONMOTEURGAUCHE, LOW);
}

void reculerMoteurDroit (int pwm) {
  analogWrite (PWMMOTEURDROIT, pwm); // Contrôle de vitesse en PWM
  digitalWrite(DIRECTIONMOTEURDROIT, HIGH);
}

void reculerMoteurGauche (int pwm) {
  analogWrite (PWMMOTEURGAUCHE, pwm); // Contrôle de vitesse en PWM
  digitalWrite(DIRECTIONMOTEURGAUCHE, HIGH);
}

void stopMoteurs() {
  analogWrite (PWMMOTEURDROIT, 0);
  digitalWrite(DIRECTIONMOTEURDROIT, LOW);
  analogWrite (PWMMOTEURGAUCHE, 0);
  digitalWrite(DIRECTIONMOTEURGAUCHE, LOW);
}


void setPwmEtDirectionMoteurs(int pwmGauche, int pwmDroit) {
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

void tournerD(float v, float percent) {
  consigneGauche =  v;
  consigneDroit  = -v * percent;
}
void tournerG(float v, float percent) {
  consigneGauche = -v * percent;
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

void tournerAngleD (float v, float angle) {
  float ang = angleTotal + angle;
  while (angleTotal<ang-0.1){
    tournerD(v, 0.75);
  }
  arreter();
}

void tournerAngleG (float v, float angle) {
  float ang = angleTotal - angle;
  while (angleTotal>ang+0.1){
    tournerG(v, 0.75);
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

    if(millis() >= derniereMAJ + tempsMAJ){
      derniereMAJ = millis();
      
      dureeTotal = millis();
      dureeMission = millis() - debutMission;

      AG = lectureCapteurUltrason(CAPTEUR_AG, 3);
      AD = lectureCapteurUltrason(CAPTEUR_AD, 3);
      CG = lectureCapteurUltrason(CAPTEUR_CG, 3);
      CD = lectureCapteurUltrason(CAPTEUR_CD, 3);
      
      actualiser_site_web(etatRobot, vitesseDroit, vitesseGauche, x, y, CG, AG, AD, CD, etatGyro, distanceTotal, dureeMission, dureeTotal);
    }
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
  if(dist<=distanceTotal){
    return 1;
  }
  return 0;
}

// -----------------------------------------------------------------------------
// PID
// -----------------------------------------------------------------------------


void runPidMoteurs(float cmdG, float cmdD) {
  if(cmdG!=ancienConsigneGauche){
    pwm_Gauche = 0;
    ancienConsigneGauche = cmdG;
  }

  if(cmdD!=ancienConsigneDroit){
    pwm_Droit = 0;
    ancienConsigneDroit = cmdD;
  }

  if (vitesseGauche < cmdG - marge)  pwm_Gauche=pwm_Gauche+5;
  else if (vitesseGauche > cmdG + marge) pwm_Gauche=pwm_Gauche-5;

  if (vitesseDroit < cmdD - marge)   pwm_Droit=pwm_Droit+5;
  else if (vitesseDroit > cmdD + marge)   pwm_Droit=pwm_Droit-5;

  // bornage
  pwm_Gauche = constrain(pwm_Gauche, -255, 255);
  pwm_Droit  = constrain(pwm_Droit,  -255, 255);

  setPwmEtDirectionMoteurs((int)pwm_Gauche, (int)pwm_Droit);
}

// -----------------------------------------------------------------------------
// GYRO
// -----------------------------------------------------------------------------

void gyro (int etat){
  if (etat == 1){
    etatGyro=1;
    digitalWrite(GYROPHARE, HIGH);
  }
  else {
    etatGyro=0;
    digitalWrite(GYROPHARE, LOW);
  }
}


// -----------------------------------------------------------------------------
// IHM
// -----------------------------------------------------------------------------

int boutonPresse(){
  int i;
  int NB_BOUTONS = 4;
  int boutons[] = { BOUTON_HAUT, BOUTON_BAS, BOUTON_CONF, BOUTON_RET };
  for(i = 0; i < NB_BOUTONS; i++){
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

void afficherEcran(char *txt1, char *txt2, char *txt3, char *txt4){
  lcd.init();
  lcd.backlight();
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

int confirmationCourrier(){
  int btn = boutonPresse();
  while(btn!=3){
    btn = boutonPresse();
  }
  return 1;
}

// -----------------------------------------------------------------------------
// SUPERVISION
// -----------------------------------------------------------------------------

void controleManuel(float vit){
  while (Serial4.available()){
    char c = Serial4.read();
    if (c == 'A') {
      avancer(vit);
    }
    else if (c == 'R') {
      reculer(vit);
    }
    else if (c == 'G') {
      tournerG(vit, 0.75);
    }
    else if (c == 'D') {
      tournerD(vit, 0.75);
    }
    else if (c == 'S') {
      arreter();
    }
    else if (c == 'B'){
      gyro(!etatGyro);
    }
  }
}

void actualiser_site_web(int etatRobot, float vitD, float vitG, float posX, float posY, float captCg, float captAg, 
                        float captAd, float captCd, int   etatGyro, float dist, float dureeMission, float dureeTotal){

  char buf[256];
  int len = snprintf(buf, sizeof(buf),
    "$ETATROBOT#%d"
    "$VITD#%.2f"
    "$VITG#%.2f"
    "$POSX#%.2f"
    "$POSY#%.2f"
    "$CAPTEUR_CG#%.2f"
    "$CAPTEUR_AG#%.2f"
    "$CAPTEUR_AD#%.2f"
    "$CAPTEUR_CD#%.2f"
    "$ETATGYRO#%d"
    "$DIST#%.2f"
    "$DUREEMISSION#%.2f"
    "$DUREETOTAL#%.2f\n",
    etatRobot, vitD, vitG, posX, posY, captCg, captAg, captAd, captCd, etatGyro, dist, dureeMission, dureeTotal);

  Serial4.write(buf, len);
}
