#define PWMMOTEURDROIT 23
#define DIRECTIONMOTEURDROIT 21

#define ENCODEURDROITA 24
#define ENCODEURDROITB 25


#define PWMMOTEURGAUCHE 22
#define DIRECTIONMOTEURGAUCHE 20

#define ENCODEURGAUCHEA 29
#define ENCODEURGAUCHEB 28

#define SPEED 100
#define TURN_SPEED 100




#define ENTRAXE 320 // A MESURER
#define NB_TIC 1560.0 // Nombre de tic par tour de roue
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


void setup() {
  Serial.begin(9600);
  initEncodeurs();
  initMoteurs();

  myTimer.begin(interruptionTimer, TIMERINTERVALE);
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


void loop(){

  tournerAngleD(90);

  if(vitesseDroit>0){
    Serial.print("D : ");
    Serial.println(vitesseDroit);
  }
  if(vitesseGauche>0){
    Serial.print("G : ");
    Serial.println(vitesseGauche);
  }

}
