#define PWMMOTEURDROIT       23
#define DIRECTIONMOTEURDROIT 21
#define PWMMOTEURGAUCHE      22
#define DIRECTIONMOTEURGAUCHE 20

#define SPEED       80
#define TURN_SPEED  60

#define TRIGGER     33
#define CAPTEUR_CG  15
#define CAPTEUR_CD  27
#define CAPTEUR_AG  31
#define CAPTEUR_AD  30

#define OBSTACLE_MAX_CM  50.0f  // portée max avant « pas d’obstacle »

const uint8_t LED_PIN = LED_BUILTIN;

// protos
float lectureCapteurUltrason(int echoPin);
void avancer(int16_t);
void reculer(int16_t);
void tournerG(int16_t);
void tournerD(int16_t);
void stopMoteurs();

void setup() {
  Serial.begin(115200);
  Serial4.begin(115200);

  pinMode(PWMMOTEURGAUCHE,      OUTPUT);
  pinMode(DIRECTIONMOTEURGAUCHE,OUTPUT);
  pinMode(PWMMOTEURDROIT,       OUTPUT);
  pinMode(DIRECTIONMOTEURDROIT, OUTPUT);

  pinMode(TRIGGER, OUTPUT);
  pinMode(CAPTEUR_CG, INPUT);
  pinMode(CAPTEUR_AG, INPUT);
  pinMode(CAPTEUR_AD, INPUT);
  pinMode(CAPTEUR_CD, INPUT);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Serial.println("Teensy prête, j'attends les commandes...");
}

void loop() {
  // 1) priorité obstacle
  float dAG = lectureCapteurUltrason(CAPTEUR_AG);
  float dAD = lectureCapteurUltrason(CAPTEUR_AD);

  if ((dAG > 0 && dAG <= OBSTACLE_MAX_CM) ||
      (dAD > 0 && dAD <= OBSTACLE_MAX_CM)) {
    // obstacle détecté : stop et LED allumée
    stopMoteurs();
    digitalWrite(LED_PIN, HIGH);
    // on repasse à l’itération suivante
    return;
  }
  // pas d’obstacle : LED éteinte
  digitalWrite(LED_PIN, LOW);

  // 2) traitement des commandes reçues
  if (Serial4.available()) {
    char c = Serial4.read();

    // blink LED court pour debug
    digitalWrite(LED_PIN, HIGH);
    delay(50);
    digitalWrite(LED_PIN, LOW);

    Serial.printf("Reçu '%c'\n", c);
    switch (c) {
      case 'A': avancer(SPEED);       break;
      case 'R': reculer(SPEED);       break;
      case 'G': tournerG(TURN_SPEED); break;
      case 'D': tournerD(TURN_SPEED); break;
      case 'S': stopMoteurs();        break;
      default:                        break;
    }
  }
}

//--- Fonctions moteurs ----------------
void avancer(int16_t p){
  analogWrite(PWMMOTEURGAUCHE, p); digitalWrite(DIRECTIONMOTEURGAUCHE, LOW);
  analogWrite(PWMMOTEURDROIT,   p); digitalWrite(DIRECTIONMOTEURDROIT,   LOW);
}
void reculer(int16_t p){
  analogWrite(PWMMOTEURGAUCHE, p); digitalWrite(DIRECTIONMOTEURGAUCHE, HIGH);
  analogWrite(PWMMOTEURDROIT,   p); digitalWrite(DIRECTIONMOTEURDROIT,   HIGH);
}
void tournerG(int16_t p){
  analogWrite(PWMMOTEURGAUCHE, p); digitalWrite(DIRECTIONMOTEURGAUCHE, HIGH);
  analogWrite(PWMMOTEURDROIT,   p); digitalWrite(DIRECTIONMOTEURDROIT,   LOW);
}
void tournerD(int16_t p){
  analogWrite(PWMMOTEURGAUCHE, p); digitalWrite(DIRECTIONMOTEURGAUCHE, LOW);
  analogWrite(PWMMOTEURDROIT,   p); digitalWrite(DIRECTIONMOTEURDROIT,   HIGH);
}
void stopMoteurs(){
  analogWrite(PWMMOTEURGAUCHE, 0); digitalWrite(DIRECTIONMOTEURGAUCHE, LOW);
  analogWrite(PWMMOTEURDROIT,   0); digitalWrite(DIRECTIONMOTEURDROIT,   LOW);
}

//--- Ultrason --------------------------
float lectureCapteurUltrason(int echoPin) {
  unsigned long duree;
  float distance_cm;

  digitalWrite(TRIGGER, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER, LOW);

  duree = pulseIn(echoPin, HIGH, 30000UL);
  distance_cm = duree * 0.034f / 2.0f;

  // si hors portée >50 cm ou timeout, on considère « pas d’obstacle »
  if (distance_cm <= 0 || distance_cm > OBSTACLE_MAX_CM) {
    distance_cm = 0.0f;
  }
  return distance_cm;
}
