#include <stdint.h>
#include <stdbool.h>
#include "billy.h"
#include "pins.h"


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
  analogWrite (PWMMOTEURDROIT, 255-pwm); // Contrôle de vitesse en PWM
  digitalWrite(DIRECTIONMOTEURDROIT, HIGH);
}

void reculerMoteurGauche (uint8_t pwm) {
  analogWrite (PWMMOTEURGAUCHE, 255-pwm); // Contrôle de vitesse en PWM
  digitalWrite(DIRECTIONMOTEURGAUCHE, HIGH);
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
