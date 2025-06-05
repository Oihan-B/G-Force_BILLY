
#define joystickPinX 
#define joystickPinY 

#define PWMMOTEURDROIT 6
#define DIRECTIONMOTEURDROIT 7
#define PWMMOTEURGAUCHE 5
#define DIRECTIONMOTEURGAUCHE 4
#define SPEED 180

void setup() {
  Serial.begin(9600);
  pinMode(DIRECTIONMOTEURDROIT, OUTPUT);
  pinMode(DIRECTIONMOTEURGAUCHE, OUTPUT);
}

int choix_joystick() {
  int valeurX = analogRead(joystickPinX);
  int valeurY = analogRead(joystickPinY);
 
  if (valeurY < 350) {
    return 1; // Avancer
  } else if (valeurY > 650) {
    return 2; // Reculer
  } else if (valeurX < 350) {
    return 3; // Tourner à gauche
  } else if (valeurX > 650) {
    return 4; // Tourner à droite
  }
  return 0; // Repos
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

void loop() {
  delay(50);

  int direction = choix_joystick();

  switch (direction) {
    case 1:
      avancer(SPEED);
      Serial.println("AVANCER");
      break;
    case 2:
      reculer(SPEED);
      Serial.println("RECULER");
      break;
    case 3:
      tournerG(SPEED);
      Serial.println("TOURNER GAUCHE");
      break;
    case 4:
      tournerD(SPEED);
      Serial.println("TOURNER DROITE");
      break;
    default:
      stopMoteurs();
      Serial.println("REPOS");
      break;
  }
}