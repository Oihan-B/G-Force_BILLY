#ifndef BILLY_H
#define BILLY_H

#include <stdio.h>
#include <stdlib.h>


// -----------------------------------------------------------------------------
// Detection Obstacles
// -----------------------------------------------------------------------------

void initCapteurUltrason();
float lectureCapteurUltrason(uint8_t capteur);
void contournerObstacle(uint8_t marge);


// -----------------------------------------------------------------------------
// Suivi Lignes
// -----------------------------------------------------------------------------

void initSuiviLigne();
char suiviLigne (); 

// A (avancer en suivant la ligne)
// D (se décaler vers la droite pour récupérer la ligne)
// G (se décaler vers la gauche pour récupérer la ligne)
// S (s'arrêter car ligne perdue)
// C (s'arrêter car checkpoint detectée)

// -----------------------------------------------------------------------------
// Commandes Moteurs
// -----------------------------------------------------------------------------


void avancerMoteurDroit(uint8_t pwm);
void avancerMoteurGauche(uint8_t pwm);
void reculerMoteurDroit (uint8_t pwm);
void reculerMoteurGauche (uint8_t pwm);
void stopMoteurs();
void avancer (uint8_t pwm);
void reculer (uint8_t pwm);
void tournerD (uint8_t pwm);
void tournerG (uint8_t pwm);


// -----------------------------------------------------------------------------
// Fonctions Odometrie
// -----------------------------------------------------------------------------

void initEncodeurs()
void interruptionTimer()
void compterDroit();
void compterGauche();

// -----------------------------------------------------------------------------
// Monitoring ESP32
// -----------------------------------------------------------------------------

void monitoring ();
/*collecte de toutes les données relatives au robot et à la mission et actualisation du site web de supervision*/

// -----------------------------------------------------------------------------
// GYRO
// -----------------------------------------------------------------------------

void gyro (uint8_t etat);


#endif
