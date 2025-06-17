#define PWMMOTEURDROIT       23
#define DIRECTIONMOTEURDROIT 21
#define ENCODEURDROITA       25
#define ENCODEURDROITB       24

#define PWMMOTEURGAUCHE      22
#define DIRECTIONMOTEURGAUCHE 20
#define ENCODEURGAUCHEA      28
#define ENCODEURGAUCHEB      29

#define SPEED   0.3
#define TURN_SPEED 0.3

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

volatile bool newSpeed = false;    // ← MODIF 1 : flag pour la boucle 20 ms

// PWM signés
volatile double pwm_Droit = 60, pwm_Gauche = 60;
float marge = 0.1;

// Consignes (signées, en rev/s)
double consigneDroit = 0, consigneGauche = 0;

void initEncodeurs() {
  pinMode(ENCODEURDROITA, INPUT);
  pinMode(ENCODEURDROITB, INPUT);
  pinMode(ENCODEURGAUCHEA, INPUT);
  pinMode(ENCODEURGAUCHEB, INPUT);
  attachInterrupt(digitalPinToInterrupt(ENCODEURDROITA), compterDroit, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODEURGAUCHEA), compterGauche, CHANGE);
}

void initMoteurs() {
  pinMode(PWMMOTEURDROIT, OUTPUT);
  pinMode(DIRECTIONMOTEURDROIT, OUTPUT);
  pinMode(PWMMOTEURGAUCHE, OUTPUT);
  pinMode(DIRECTIONMOTEURGAUCHE, OUTPUT);
}

void avancerMoteurDroit(uint8_t pwm) {
  analogWrite(PWMMOTEURDROIT, pwm);
  digitalWrite(DIRECTIONMOTEURDROIT, LOW);
}
void reculerMoteurDroit(uint8_t pwm) {
  analogWrite(PWMMOTEURDROIT, pwm);
  digitalWrite(DIRECTIONMOTEURDROIT, HIGH);
}
void avancerMoteurGauche(uint8_t pwm) {
  analogWrite(PWMMOTEURGAUCHE, pwm);
  digitalWrite(DIRECTIONMOTEURGAUCHE, LOW);
}
void reculerMoteurGauche(uint8_t pwm) {
  analogWrite(PWMMOTEURGAUCHE, pwm);
  digitalWrite(DIRECTIONMOTEURGAUCHE, HIGH);
}
void stopMoteurs() {
  analogWrite(PWMMOTEURDROIT, 0);
  analogWrite(PWMMOTEURGAUCHE, 0);
  digitalWrite(DIRECTIONMOTEURDROIT, LOW);
  digitalWrite(DIRECTIONMOTEURGAUCHE, LOW);
}

void setPwmEtDirectionMoteurs(int16_t pd, int16_t pg) {
  if (pd > 0)        avancerMoteurDroit(pd);
  else if (pd < 0)   reculerMoteurDroit(-pd);

  if (pg > 0)        avancerMoteurGauche(pg);
  else if (pg < 0)   reculerMoteurGauche(-pg);

  if (pd == 0 && pg == 0) stopMoteurs();
}

// --- on ne change plus runPidMoteurs() ici ---

void interruptionTimer() {
  // ← MODIF 2 : calcul de la vitesse en rev/s
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
  newSpeed = true;  // ← MODIF 3 : on autorise la boucle de régul
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

// --- On remplace les runPidMoteurs() dans avancer/reculer par un simple réglage de consigne ---

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

void runPidMoteurs(float cmdG, float cmdD) {
  // inchangé
  if (vitesseGauche < cmdG - marge)  pwm_Gauche++;
  else if (vitesseGauche > cmdG + marge) pwm_Gauche--;

  if (vitesseDroit < cmdD - marge)   pwm_Droit++;
  else if (vitesseDroit > cmdD + marge)   pwm_Droit--;

  // bornage
  pwm_Gauche = constrain(pwm_Gauche, -255, 255);
  pwm_Droit  = constrain(pwm_Droit,  -255, 255);

  setPwmEtDirectionMoteurs((int)pwm_Droit, (int)pwm_Gauche);
}

void setup() {
  Serial.begin(115200);
  initEncodeurs();
  initMoteurs();
  myTimer.begin(interruptionTimer, TIMERINTERVALE);
}

void loop(){
  if (distanceTotal < 1000.0) {
    // on positionne la consigne
    avancer(SPEED);
    // --- MODIF 4 : on ne régule qu’après la mesure 20 ms ---
    if (newSpeed) {
      newSpeed = false;
      runPidMoteurs(consigneGauche, consigneDroit);
    }
  } else {
    stopMoteurs();
    while(1);  // blocage une fois l’objectif atteint
  }
}
