#define PWMMOTEURDROIT 23
#define DIRECTIONMOTEURDROIT 21

#define PWMMOTEURGAUCHE 22
#define DIRECTIONMOTEURGAUCHE 20

#define ENCODEURDROITA 25
#define ENCODEURDROITB 24

#define ENCODEURGAUCHEA 28
#define ENCODEURGAUCHEB 29

#define SPEED   0.3

#define ENTRAXE 320     // mm
#define NB_TIC  840.0   // ticks/tour
#define D_ROUE  100     // mm de diamètre

IntervalTimer myTimer;
#define TIMERINTERVALE 20000  // µs = 20 ms

volatile double pi = 3.14159;

volatile int16_t compteDroit = 0, compteGauche = 0;
volatile double distDroit = 0, distGauche = 0;
volatile double distMoy = 0;
volatile double vitesseDroit = 0, vitesseGauche = 0;
volatile double distanceTotal = 0; // mm
volatile double angleTotal = 0;    // rad
volatile double x = 0, y = 0;


// PWM signés
volatile double pwm_Droit = 60, pwm_Gauche = 60;
float marge = 0.1;

// Consignes (signées, en rev/s)
double consigneDroit = 0, consigneGauche = 0;
double ancienConsigneDroit = 0, ancienConsigneGauche = 0;

#define S1 2
#define S2 3
#define S3 4
#define S4 5
#define S5 6

#define TRIGGER 33
#define CAPTEUR_CG 15
#define CAPTEUR_CD 27
#define CAPTEUR_AG 31
#define CAPTEUR_AD 30

int capteurs[5] = {S1, S2, S3, S4, S5};
int capteurs_ultrasons[4] = {CAPTEUR_CG, CAPTEUR_AG, CAPTEUR_AD, CAPTEUR_CD};
int boutons[4] = {0, 1, 7, 8};

void initMoteurs () {
  pinMode(PWMMOTEURDROIT, OUTPUT);
  pinMode(PWMMOTEURGAUCHE, OUTPUT);
  pinMode(DIRECTIONMOTEURDROIT, OUTPUT);
  pinMode(DIRECTIONMOTEURGAUCHE, OUTPUT);
}

void initEncodeurs() {
  myTimer.begin(interruptionTimer, TIMERINTERVALE);
  pinMode(ENCODEURDROITA, INPUT);
  pinMode(ENCODEURDROITB, INPUT);
  pinMode(ENCODEURGAUCHEA, INPUT);
  pinMode(ENCODEURGAUCHEB, INPUT);
  attachInterrupt(digitalPinToInterrupt(ENCODEURDROITA), compterDroit, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODEURGAUCHEA), compterGauche, CHANGE);
}

void setup() {
  Serial.begin(9600);
  initMoteurs();
  initEncodeurs();

  int i;

  for (i = 0; i < 5; i++) {
    pinMode(capteurs[i], INPUT);
  }
  for (i = 0; i < 4; i++) {
    pinMode(boutons[i], INPUT_PULLUP);
  }
  for (i = 0; i < 4; i++) {
    pinMode(capteurs_ultrasons[i], INPUT);
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

void avancer(float v) {
  consigneDroit = consigneGauche = v;
}
void reculer(float v) {
  consigneDroit = consigneGauche = -v;
}
void tournerD(float v) {
  consigneGauche =  v;
  consigneDroit  = -v*0.75;
}
void tournerG(float v) {
  consigneGauche = -v*0.75;
  consigneDroit  =  v;
}

void tournerDsoft(float v, float percent) {
  consigneGauche = (1+2*percent)* v;
  consigneDroit  = 0*v;
}

void tournerGsoft(float v, float percent) {
  consigneGauche = 0*v;
  consigneDroit  = (1+2*percent)*v;
}

void arreter(){
  consigneGauche = consigneDroit = 0;
}


void setPwmEtDirectionMoteurs(int16_t pwmGauche, int16_t pwmDroit) {
  if (pwmDroit > 0)        avancerMoteurDroit(pwmDroit);
  else if (pwmDroit < 0)   reculerMoteurDroit(-pwmDroit);

  if (pwmGauche > 0)        avancerMoteurGauche(pwmGauche);
  else if (pwmGauche < 0)   reculerMoteurGauche(-pwmGauche);

  if (pwmDroit == 0 && pwmGauche == 0) stopMoteurs();
}



void interruptionTimer() {
  vitesseDroit  = (double)compteDroit  * (1.0 / 0.02) / NB_TIC;
  vitesseGauche = (double)compteGauche * (1.0 / 0.02) / NB_TIC;

  // odométrie
  distDroit  = (pi * D_ROUE) / NB_TIC * compteDroit;
  distGauche = (pi * D_ROUE) / NB_TIC * compteGauche;
  distMoy    = (distDroit + distGauche) * 0.5;
  distanceTotal += distMoy;

  double delta = (distDroit - distGauche) / ENTRAXE;
  angleTotal += delta;
  if (angleTotal > pi)  angleTotal -= 2*pi;
  if (angleTotal < -pi) angleTotal += 2*pi;
  x += distMoy * cos(angleTotal);
  y += distMoy * sin(angleTotal);

  compteDroit = compteGauche = 0;
  runPidMoteurs(consigneGauche, consigneDroit);
}

void compterDroit() {
  if (digitalRead(ENCODEURDROITA) == digitalRead(ENCODEURDROITB))
    compteDroit++;
  else
    compteDroit--;
}
void compterGauche() {
  if (digitalRead(ENCODEURGAUCHEA) == digitalRead(ENCODEURGAUCHEB))
    compteGauche--;
  else
    compteGauche++;
}


void runPidMoteurs(float cmdG, float cmdD) {
  if(cmdG!=ancienConsigneGauche){
    pwm_Gauche = 0;
    ancienConsigneGauche = cmdG;
  }

  if(cmdD!=ancienConsigneDroit){
    pwm_Droit = 0;
    ancienConsigneDroit = cmdD;
  }

  if (vitesseGauche < cmdG - marge)  pwm_Gauche=pwm_Gauche+3;
  else if (vitesseGauche > cmdG + marge) pwm_Gauche=pwm_Gauche-3;

  if (vitesseDroit < cmdD - marge)   pwm_Droit=pwm_Droit+3;
  else if (vitesseDroit > cmdD + marge)   pwm_Droit=pwm_Droit-3;

  // bornage
  pwm_Gauche = constrain(pwm_Gauche, -255, 255);
  pwm_Droit  = constrain(pwm_Droit,  -255, 255);

  setPwmEtDirectionMoteurs((int)pwm_Gauche, (int)pwm_Droit);
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

    for (i = 0; i < 5; i++){
      detections[i] = digitalRead(capteurs[i]); // 0 SOL // 1 LIGNE
    }

    detections[3] = 1;
    detections[4] = 1;

    for (i = 0; i < 5; i++){
      Serial.println(detections[i]);
    }

    if (!detections[0] && !detections[1] && !detections[2]) {
      Serial.println("\nCheckpoint detecte\n");
      arreter();
    }

    else if (!detections[2]) {
      Serial.println("\nTourner à droite pour retrouver la ligne\n");
      tournerD(SPEED);
    } 

    else if (!detections[0]) {
      Serial.println("\nTourner à gauche pour retrouver la ligne\n");
      tournerG(SPEED);
    } 

    else if (!detections[1]) {
      Serial.println("\nAvancer tout droit\n");
      avancer(SPEED);
    }

    else {
      Serial.println("\nLigne perdue : STOP ou reculer\n");
      arreter();
    }
  }
}
