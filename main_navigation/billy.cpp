#include <stdint.h>
#include <stdbool.h>
#include "billy.h"
#include "pins.h"

// -----------------------------------------------------------------------------
// Suivi Lignes
// -----------------------------------------------------------------------------

int lectureCapteurLigne(int capteur_id) {
    return analogRead(capteur_id) ;
}

void calibrationSuiviLignes(int seuils[5]) {
    int mesures[5][100]; //100 mesures pour chaque capteur
	int i, c;
	
    for (i = 0; i < 100; i++) {
        for (c = 0; c < 5; c++) {
            mesures[c][i] = readCapteur(c);
        }
    }

    for (c = 0; c < 5; c++) {
        int min = mesures[c][0];
        int max = mesures[c][0];
        for (i = 1; i < 100; i++) {
            if (mesures[c][i] < min)
                min = mesures[c][i];
            if (mesures[c][i] > max)
                max = mesures[c][i];
        }
        seuils[c] = (max + min) / 2;
        printf("Capteur %d : min=%d, max=%d, seuil=%d\n", c, min, max, seuils[c]);
    }
}

// -----------------------------------------------------------------------------
// Commandes Moteurs
// -----------------------------------------------------------------------------
void avancerMoteurDroit(uint8_t pwm) {
  analogWrite (PWMMOTEURDROIT, pwm); // Contrôle de vitesse en PW
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

void avancer (uint8_t pwm){
  avancerMoteurGauche(pwm);
  avancerMoteurDroit(pwm);
}

void reculer (uint8_t pwm){
  reculerMoteurGauche(pwm);
  reculerMoteurDroit(pwm);
}

void tournerD (uint8_t pwm){
  avancerMoteurGauche(pwm);
  reculerMoteurDroit(pwm);
}

void tournerG (uint8_t pwm){
  reculerMoteurGauche(pwm);
  avancerMoteurDroit(pwm);
}

// -----------------------------------------------------------------------------
// Fonctions Odometrie
// -----------------------------------------------------------------------------

void compterDroit() {
  
  if(digitalRead(ENCODEURDROITA) == digitalRead(ENCODEURDROITB)){
    compteDroit++;
  }else {
    compteDroit--;
  }
  distDroit=((pi*D_roue)/nb_tic)*compteDroit;
}

void compterGauche() {
  if(digitalRead(ENCODEURGAUCHEA) == digitalRead(ENCODEURGAUCHEB)){
    compteGauche--;
  }else{
    compteGauche++;
  }
  distGauche=((pi*D_roue)/nb_tic)*compteGauche;
}
