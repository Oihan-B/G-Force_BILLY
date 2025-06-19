#include <LiquidCrystal_I2C.h>
#ifndef BILLY_H
#define BILLY_H


// -----------------------------------------------------------------------------
// Detection Obstacles
// -----------------------------------------------------------------------------

void initCapteurUltrason();
float lectureCapteurUltrason(int capteur, int size);
void contournerObstacle();


// -----------------------------------------------------------------------------
// Suivi Lignes
// -----------------------------------------------------------------------------

void initSuiviLigne();
char suiviLigne (); 

// -----------------------------------------------------------------------------
// Commandes Moteurs
// -----------------------------------------------------------------------------

void initMoteurs();
void avancerMoteurDroit(int pwm);
void avancerMoteurGauche(int pwm);
void reculerMoteurDroit (int pwm);
void reculerMoteurGauche (int pwm);
void stopMoteurs();
void avancer (float v);
void reculer (float v);
void tournerD (float v, float percent);
void tournerG (float v, float percent);
void tournerDsoft(float v, float percent);
void tournerGsoft(float v, float percent);
void arreter();
void tournerAngleD (float angle);
void tournerAngleG (float angle);


// -----------------------------------------------------------------------------
// Fonctions Odometrie
// -----------------------------------------------------------------------------

void initEncodeurs();
void interruptionTimer();
void compterDroit();
void compterGauche();
int distanceAtteinte(int dist);

// -----------------------------------------------------------------------------
// PID
// -----------------------------------------------------------------------------

void runPidMoteurs(float cmdG, float cmdD);

// -----------------------------------------------------------------------------
// GYRO
// -----------------------------------------------------------------------------

void gyro (int etat);

// -----------------------------------------------------------------------------
// IHM
// -----------------------------------------------------------------------------

extern LiquidCrystal_I2C lcd;
int boutonPresse();
void afficherEcran(char *txt1, char *txt2, char *txt3, char *txt4);

// -----------------------------------------------------------------------------
// SCENARIOS
// -----------------------------------------------------------------------------

void scenario_1(float dist, float consigne_vitesse);
void scenario_2(float consigne_vitesse);
void scenario_3(float consigne_vitesse);

#endif
