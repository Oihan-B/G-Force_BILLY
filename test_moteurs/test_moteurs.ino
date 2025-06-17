//#include <PID_v1.h>

#define PWMMOTEURDROIT 23
#define DIRECTIONMOTEURDROIT 21

#define ENCODEURDROITA 25
#define ENCODEURDROITB 24


#define PWMMOTEURGAUCHE 22
#define DIRECTIONMOTEURGAUCHE 20

#define ENCODEURGAUCHEA 28
#define ENCODEURGAUCHEB 29

#define SPEED 0.3
#define TURN_SPEED 0.3




#define ENTRAXE 320 // A MESURER
#define NB_TIC 840.0 // Nombre de tic par tour de roue
#define D_ROUE 100 // Diametre roue



IntervalTimer myTimer;
#define TIMERINTERVALE 20000 // ms

volatile double pi=3.14;

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

float marge = 0.05;




void initEncodeurs() {
  pinMode(ENCODEURDROITA, INPUT);
  pinMode(ENCODEURDROITB, INPUT);
  pinMode(ENCODEURGAUCHEA, INPUT);
  pinMode(ENCODEURGAUCHEB, INPUT);
  attachInterrupt(digitalPinToInterrupt(ENCODEURDROITA), compterDroit, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODEURGAUCHEA), compterGauche, CHANGE);
}

void initMoteurs () {
  pinMode(PWMMOTEURDROIT, OUTPUT);
  pinMode(PWMMOTEURGAUCHE, OUTPUT);
  pinMode(DIRECTIONMOTEURDROIT, OUTPUT);
  pinMode(DIRECTIONMOTEURGAUCHE, OUTPUT);
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


void setPwmEtDirectionMoteurs (int16_t pwmMoteurDroit, int16_t pwmMoteurGauche) {
  if(pwmMoteurDroit>0){
    avancerMoteurDroit(pwmMoteurDroit);
  }else if(pwmMoteurDroit<0){
    reculerMoteurDroit(-pwmMoteurDroit);
  }
  if(pwmMoteurGauche>0){
    avancerMoteurGauche(pwmMoteurGauche);
  }else if(pwmMoteurGauche<0){
    reculerMoteurGauche(-pwmMoteurGauche);
  }
  if(pwmMoteurDroit==0 && pwmMoteurGauche==0){
    stopMoteurs();
  }
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


void tournerAngleD (uint8_t angle) {
  while (angleTotal<angle-0.1){
    tournerD(TURN_SPEED);
  }
  stopMoteurs();
}

void tournerAngleG (uint8_t angle) {
  while (angleTotal>angle+0.1){
    tournerG(TURN_SPEED);
  }
  stopMoteurs();
}


// -----------------------------------------------------------------------------
// PID
// -----------------------------------------------------------------------------

double consigneDroit = 0;
double consigneGauche = 0;
 
double Kp = 200;
double Ki = 100;
double Kd = 0;
 
PID pidDroit(&vitesseDroit, &pwm_Droit, &consigneDroit, Kp, Ki, Kd, DIRECT);
PID pidGauche(&vitesseGauche, &pwm_Gauche, &consigneGauche, Kp, Ki, Kd, DIRECT);

void initPid() {
  pidDroit.SetMode(AUTOMATIC);
  pidDroit.SetOutputLimits(-255, 255);
  pidGauche.SetMode(AUTOMATIC);
  pidGauche.SetOutputLimits(-255, 255);
}

void runPidMoteurs ( float commandeMoteurGauche, float commandeMoteurDroit) {
    /*
    consigneDroit = commandeMoteurDroit;
    consigneGauche = commandeMoteurGauche;
    pidDroit.Compute();
    pidGauche.Compute();
    setPwmEtDirectionMoteurs((int)pwm_Droit, (int)pwm_Gauche);
    */

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
    angleTotal+=2*theta;
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


void setup() {
  Serial.begin(9600);
  initEncodeurs();
  initMoteurs();
  //initPid();

  myTimer.begin(interruptionTimer, TIMERINTERVALE);
}

void loop(){

  //tournerD(SPEED);

  if(distanceTotal<1000){
    avancer(SPEED);
  }else{
    stopMoteurs();
  }
  /*Serial.print("Consigne : ");
  Serial.println(consigneDroit);
  Serial.println(pwm_Droit);
  /*if(vitesseDroit>0){
    Serial.print("D : ");
    Serial.println(vitesseDroit);
  }
  if(vitesseGauche>0){
    Serial.print("G : ");
    Serial.println(vitesseGauche);
  }*/
  

  

}
