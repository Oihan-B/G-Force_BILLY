#define PWMMOTEURDROIT 23
#define DIRECTIONMOTEURDROIT 21

#define PWMMOTEURGAUCHE 22
#define DIRECTIONMOTEURGAUCHE 20

#define SPEED 100
#define TURN_SPEED 100

void setup() {
  Serial.begin(9600);
  pinMode(DIRECTIONMOTEURDROIT, OUTPUT);
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

void loop(){

  stopMoteurs();
  delay(1000);

  avancer(SPEED);
  delay(2000);
  stopMoteurs();
  delay(1000);

  tournerD(SPEED);
  delay(2000);
  stopMoteurs();
  delay(1000);

  tournerG(SPEED);
  delay(2000);
  stopMoteurs();
  delay(1000);

  reculer(SPEED);
  delay(2000);
  stopMoteurs();
  delay(1000);
}