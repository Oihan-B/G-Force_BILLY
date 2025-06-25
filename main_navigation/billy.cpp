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
volatile float angleTotal = 0; // radian
volatile double x = 0; //mm 
volatile double y = 0; //mm
volatile double theta = 0; // radian entre -Pi et Pi

float marge = 0.1;

double ancienConsigneDroit = 0;
double ancienConsigneGauche = 0;
double consigneDroit = 0;
double consigneGauche = 0;

int capteurs_ultrasons[4] = {CAPTEUR_CG, CAPTEUR_AG, CAPTEUR_AD, CAPTEUR_CD};
float CG = 0;
float AG = 0;
float AD = 0;
float CD = 0;

volatile float echoStartCG = 0;
volatile float echoStartAG = 0;
volatile float echoStartAD = 0;
volatile float echoStartCD = 0;

volatile float startCG = 0;
volatile float startAG = 0;
volatile float startAD = 0;
volatile float startCD = 0;

float dureeTotal;
float dureeMission;
float debutMission;

double derniereLectureUltrason = 0;
double tempsLectureUltrason = 50;
double derniereMAJ = 0;
double tempsMAJ = 2000;

int etatRobot = 0;
int etatGyro = 0;

char buf[32];

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
  analogWrite (PWMMOTEURDROIT, pwm); 
  digitalWrite(DIRECTIONMOTEURDROIT, LOW);
}

void avancerMoteurGauche(int pwm) {
  analogWrite (PWMMOTEURGAUCHE, pwm); 
  digitalWrite(DIRECTIONMOTEURGAUCHE, LOW);
}

void reculerMoteurDroit (int pwm) {
  analogWrite (PWMMOTEURDROIT, pwm); 
  digitalWrite(DIRECTIONMOTEURDROIT, HIGH);
}

void reculerMoteurGauche (int pwm) {
  analogWrite (PWMMOTEURGAUCHE, pwm); 
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

void tournerD(float v, float coeff) {
  consigneGauche =  v;
  consigneDroit  = -v * coeff;
}

void tournerG(float v, float coeff) {
  consigneGauche = -v * coeff;
  consigneDroit  =  v;
}

void tournerDsoft(float v, float coeff) {
  consigneGauche = v + (coeff * v);
  consigneDroit  = v - (coeff * v);
}

void tournerGsoft(float v, float coeff) {
  consigneGauche = v - (coeff * v);
  consigneDroit  = v + (coeff * v);
}

void arreter(){
  consigneGauche = consigneDroit = 0;
}

void tournerAngleD (float v, float coeff, float angle) {
  v = 0.8 * v;
  float ang = angleTotal - angle * 0.95;
  tournerD(v, coeff);
  while (angleTotal > ang + 0.35){
    yield();
  }
  arreter();
}

void tournerAngleG (float v, float coeff, float angle) {
  v = 0.8 * v;
  float ang = angleTotal + angle * 0.95;
  tournerG(v, coeff);
  while (angleTotal < ang - 0.35){
    yield();
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
    vitesseDroit = ((50 * compteDroit) / (NB_TIC));
    vitesseGauche = ((50 * compteGauche) / (NB_TIC));
    
    distMoy = (distDroit+distGauche) / 2;
    distanceTotal += distMoy;

    if(distDroit>distGauche || distGauche>distDroit){
      dist = distDroit - distGauche;
      theta = dist / ENTRAXE;
    }

    angleTotal += theta * 2;

    /*
    if(angleTotal>pi){
      angleTotal-=2*pi;
    }else if(angleTotal<-pi){
      angleTotal+=2*pi;
    }*/

    x += distMoy * cos(angleTotal);
    y += distMoy * sin(angleTotal);
    compteDroit = 0;
    compteGauche = 0;
    theta = 0;
    
    runPidMoteurs(consigneGauche, consigneDroit);

    if ((millis() >= derniereLectureUltrason + tempsLectureUltrason) && etatRobot != 0){
      digitalWrite(TRIGGER, LOW);
      delayMicroseconds(2);
      digitalWrite(TRIGGER, HIGH);
      delayMicroseconds(10);
      digitalWrite(TRIGGER, LOW);
      derniereLectureUltrason = millis();
    }

    if(millis() >= derniereMAJ + tempsMAJ){
      derniereMAJ = millis();
      
      dureeTotal = millis();
      dureeMission = millis() - debutMission;
      
      actualiserSiteWeb(etatRobot, vitesseDroit, vitesseGauche, x, y, CG, AG, AD, CD, etatGyro, distanceTotal, dureeMission, dureeTotal);
    }
}

void compterDroit() {
  if(digitalRead(ENCODEURDROITA) == digitalRead(ENCODEURDROITB)){
    compteDroit++;
  }else {
    compteDroit--;
  }
  distDroit = compteDroit * ((pi * D_ROUE) / NB_TIC);
}

void compterGauche() {
  if(digitalRead(ENCODEURGAUCHEA) == digitalRead(ENCODEURGAUCHEB)){
    compteGauche--;
  }else{
    compteGauche++;
  }
  distGauche = compteGauche * ((pi * D_ROUE) / NB_TIC);
}

void avancerDist(float vit, float dist){
  float d = distanceTotal + (dist*5)/11;
  avancer(vit);
  while(distanceTotal < d){
    yield();
  }
  arreter();
}

int distanceAtteinte(int dist){
  if((dist)/2<=distanceTotal){
    return 1;
  }
  return 0;
}

// -----------------------------------------------------------------------------
// PID
// -----------------------------------------------------------------------------

void runPidMoteurs(float cmdG, float cmdD) {
  int maxPWM = 200;

  if(cmdG! = ancienConsigneGauche){
    pwm_Gauche = 0;
    ancienConsigneGauche = cmdG;
  }

  if(cmdD != ancienConsigneDroit){
    pwm_Droit = 0;
    ancienConsigneDroit = cmdD;
  }

  if (vitesseGauche < cmdG - marge)  pwm_Gauche = pwm_Gauche + 5;
  else if (vitesseGauche > cmdG + marge) pwm_Gauche = pwm_Gauche - 5;

  if (vitesseDroit < cmdD - marge)   pwm_Droit = pwm_Droit + 5;
  else if (vitesseDroit > cmdD + marge)   pwm_Droit = pwm_Droit - 5;

  pwm_Gauche = constrain(pwm_Gauche, -maxPWM, maxPWM);
  pwm_Droit  = constrain(pwm_Droit,  -maxPWM, maxPWM);

  setPwmEtDirectionMoteurs((int)pwm_Gauche, (int)pwm_Droit);
}

// -----------------------------------------------------------------------------
// Detection Obstacles
// -----------------------------------------------------------------------------

void initCapteurUltrason(){
  int i;

  for (i = 0; i < 4; i++) {
    pinMode(capteurs_ultrasons[i], INPUT_PULLUP);
  }
  pinMode(TRIGGER, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(CAPTEUR_CG), lectureUSCG, CHANGE);
  attachInterrupt(digitalPinToInterrupt(CAPTEUR_AG), lectureUSAG, CHANGE);
  attachInterrupt(digitalPinToInterrupt(CAPTEUR_AD), lectureUSAD, CHANGE);
  attachInterrupt(digitalPinToInterrupt(CAPTEUR_CD), lectureUSCD, CHANGE);
}

void lectureUSCG(){
  if(digitalRead(CAPTEUR_CG) == HIGH){
    echoStartCG = micros();
    startCG = 1;
  }else{
    if(startCG){
      unsigned long dureeCG = micros() - echoStartCG;
      CG = dureeCG * 0.034f / 2.0f;
      if (CG < 8 || CG > 50){ 
        CG = 0;
      }
      startCG = 0;
    }
  }
}

void lectureUSAG(){
  if(digitalRead(CAPTEUR_AG) == HIGH){
    echoStartAG = micros();
    startAG = 1;
  }else{
    if(startAG){
      unsigned long dureeAG = micros() - echoStartAG;
      AG = dureeAG * 0.034f / 2.0f;
      if (AG < 8 || AG > 50){ 
        AG = 0;
      }
      startAG = 0;
    }
  }
}

void lectureUSAD(){
  if(digitalRead(CAPTEUR_AD) == HIGH){
    echoStartAD = micros();
    startAD = 1;
  }else{
    if(startAG){
      unsigned long dureeAD = micros() - echoStartAD;
      AD = dureeAD * 0.034f / 2.0f;
      if (AD < 8 || AD > 50){ 
        AD = 0;
      }
      startAD = 0;
    }
  }
}

void lectureUSCD(){
  if(digitalRead(CAPTEUR_CD) == HIGH){
    echoStartCD = micros();
    startCD = 1;
  }else{
    if(startCD){
      unsigned long dureeCD = micros() - echoStartCD;
      CD = dureeCD * 0.034f / 2.0f;
      if (CD < 8 || CD > 50){ 
        CD = 0;
      }
      startCD = 0;
    }
  }
}

void lectureCapteurUltrason() { // PLUS UTILISE CAR TROP BLOQUANT

  unsigned long duree;
  float detection;
  int i;

  for (i = 0; i < 4; i++){

    detection = 0;

    digitalWrite(TRIGGER, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIGGER, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIGGER, LOW);

    duree = pulseIn(capteurs_ultrasons[i], HIGH, 30000UL);

    detection = duree * 0.034f / 2.0f;

    delay(10);

    if (detection < 8 || detection > 50){ 
      detection = 0;
    }
    if (i == 0){
      CG = detection;
    }
    else if (i == 1){
      AG = detection;
    }
    else if (i == 2){
      AD = detection;
    }
    else {
      CD = detection;
    }
  }
}

void contournerObstacle(float vit) {
  tournerAngleG(vit, 1, pi/2); // Tourner à gauche de 90° pour éviter l'obstacle
  delay(1000);
  avancerDist(vit, 500); // On avance manuellement pour être sûr que notre capteur droit récupère l'obstacle
  avancer(vit); // Avancer pour s'éloigner de l'obstacle
  while (CD != 0) { // On avance tant qu'on détecte l'obstacle
    yield();
  }
  //avancerDist(vit, 100);
  tournerAngleD(vit, 1, pi/2); // On peut tourner à droit directement car le virage se fait côté extérieur, pas de risque que l'arrière du robot touche
  delay(1000);
  avancerDist(vit, 500);
  avancer(vit); // Avancer pour s'éloigner de l'obstacle
  while (CD != 0) { // On avance tant qu'on détecte l'obstacle
    yield();
  }
  avancerDist(vit, 250); // Une fois avoir dépassé l'obstacle quand on le longe de manière parallèle, on avance d'une grande distance de sécurité 
                        // pour couvrir l'arrière du robot et anticiper le virage droit puis le virage intérieur gauche pour récupérer la ligne en parallèle
  tournerAngleD(vit, 1, pi/2); // Tourner à droite pour longer le dernier côté de l'obstacle avant de récuperer la ligne
  delay(1000);
  avancer(vit); // Avancer pour s'éloigner de l'obstacle
  while (suiviLigne() == 'S') { // On avance tant qu'on retrouve pas la ligne perpendiculairement
    yield();
  }
  avancerDist(vit, 50); // On avance légèrement pour que le prochain virage positionne le centre de l'avant de BILLY directement sur la ligne
  tournerAngleG(vit, 1, pi/2); // Revenir à la trajectoire initiale
  delay(1000);
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

char suiviLigne(){ 
  int capteurs[5] = {S5, S4, S3, S2, S1};
  int detections[5];
  int i;

  for (i = 0; i < 5; i++){
    detections[i] = digitalRead(capteurs[i]); // 0 SOL // 1 LIGNE
  }

  if (detections[1] && detections[2] && detections[3]) {
    return 'C';
    envoyerLogs("\nJ'ai detecté un arrêt, p'tite pause chill !");
  }

  else if (detections[2]) {
    return 'A';
  } 

  else if (detections[3] || detections[4]) {
    return 'D';
  } 

  else if (detections[0] || detections[1]) {
    return 'G';
  } 

  else {
    return 'S';
    envoyerLogs("\nOups, j'ai perdu la ligne, désolé !");
  }
}

// -----------------------------------------------------------------------------
// GYRO
// -----------------------------------------------------------------------------

void gyro (int etat){
  if (etat == 1){
    etatGyro = 1;
    digitalWrite(GYROPHARE, HIGH);
  }
  else {
    etatGyro = 0;
    digitalWrite(GYROPHARE, LOW);
  }
}

// -----------------------------------------------------------------------------
// IHM
// -----------------------------------------------------------------------------

int boutonPresse(){
  int i;
  int NB_BOUTONS = 4;
  int boutons[] = {BOUTON_HAUT, BOUTON_BAS, BOUTON_CONF, BOUTON_RET };
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

void afficherEcran(int duree, const char *txt1, const char *txt2, const char *txt3, const char *txt4){
  envoyerLogs("\nActualisation de l'écran LCD :");
  envoyerLogs(txt1);
  envoyerLogs("\n");
  envoyerLogs(txt2);
  envoyerLogs("\n");
  envoyerLogs(txt3);
  envoyerLogs("\n");
  envoyerLogs(txt4);

  lcd.clear();
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
  if (duree != 0){
    delay(duree);
    lcd.clear();
  }
}

void envoyerLogs(const char *log){
  Serial4.write(log);
  Serial4.write("\n");
  delayMicroseconds(50); // S'assurer que les logs s'envoient correctement
}

int confirmationCourrier(){
  afficherEcran(0, "Salut, c'est BILLY !", "Confirme que tu", "as recu ton colis.", "Merci !");
  int btn = boutonPresse();
  float tempsAttente = millis();
  int aff = 0;
  while(btn != 7){
    btn = boutonPresse();
    if((millis() - tempsAttente > 60000) && aff == 0){ // Easter Egg du projet BILLY, l'utilisateur n'a que 60 secondes pour récuperer le colis avant de se prendre un coup de pression
      afficherEcran(0, "AUTODESTRUCTION", "EN", "COURS !  ", "3, 2, 1 ...");
      aff = 1;
    }
  }
  afficherEcran(2000, "Merci a toi !", "Moi j'me barre.", "Ciaoooo LOOSER !", "");
  return 1;
}

void lancerMission(int s, float t){
  envoyerLogs("\nLancement d'une nouvelle mission !");
  envoyerLogs("\nStatut du robot et durée mission mises à jour.");
  etatRobot = s;
  debutMission = t;
}

// -----------------------------------------------------------------------------
// SUPERVISION
// -----------------------------------------------------------------------------

void controleManuel(float vit){
  while (Serial4.available()){

    char c = Serial4.read();

    while (c != '}'){

      if (Serial4.available()){
        c = Serial4.read();
      }

      if (AG != 0 || AD != 0){
        envoyerLogs("\nObstacle detectée, interruption automatique du contrôle manuel !"); // L'utilisateur peut quand même forcer et reprendre le contrôle
        arreter();
        gyro(1);
      }
  
      if (c == 'A') {
        avancer(vit);
      }
      else if (c == 'R') {
        reculer(vit);
      }
      else if (c == 'G') {
        tournerG(vit, 0.6);
      }
      else if (c == 'D') {
        tournerD(vit, 0.6);
      }
      else if (c == 'S') {
        arreter();
      }
      else if (c == 'B'){
        gyro(!etatGyro);
      }
      yield();
    }
    break;
  }
}

void actualiserSiteWeb(int etatRobot, float vitD, float vitG, float posX, float posY, float captCg, float captAg, 
                        float captAd, float captCd, int   etatGyro, float dist, float dureeMission, float dureeTotal){

  char buf[256];
  int len = snprintf(buf, sizeof(buf),
    "\n$ETATROBOT#%d"
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
    etatRobot, vitD, vitG, posX/1000, posY/1000, 
    captCg, captAg, captAd, captCd, 
    etatGyro, dist/1000, dureeMission/1000, 
    dureeTotal);

  Serial4.write(buf, len);
}
// 666 le nombre du diable bouahhahaa