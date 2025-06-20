#include <LiquidCrystal_I2C.h>
#ifndef BILLY_H
#define BILLY_H

// -----------------------------------------------------------------------------
// VARIABLES
// -----------------------------------------------------------------------------

#define SPEED 0.3

#define ENTRAXE 320 
#define NB_TIC 1560.0 // Nombre de tic par tour de roue
#define D_ROUE 100 // Diametre roue
#define TIMERINTERVALE 20000 // ms

extern IntervalTimer myTimer;

extern volatile double pi;
extern volatile double compteDroit;
extern volatile double compteGauche;
extern volatile double dist;
extern volatile double distMoy;
extern volatile double distDroit;
extern volatile double distGauche;
extern volatile double vitesseDroit;
extern volatile double vitesseGauche;
extern volatile double pwm_Droit;
extern volatile double pwm_Gauche;
extern volatile double intervalle;
extern volatile double distanceTotal;  // mm
extern volatile double angleTotal;     // radians
extern volatile double x;              // mm
extern volatile double y;              // mm
extern volatile double theta;          // radian (-π … +π)

extern float  marge;
extern double ancienConsigneDroit;
extern double ancienConsigneGauche;
extern double consigneDroit;
extern double consigneGauche;

void startMission(int s, float t);

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

// -----------------------------------------------------------------------------
// SUPERVISION
// -----------------------------------------------------------------------------

void controleManuel(float vit);
void actualiser_site_web(int etatRobot, float vitD, float vitG, float posX, float posY, float CAPTEUR_CG, float CAPTEUR_AG, 
                        float CAPTEUR_AD, float CAPTEUR_CD, int etatGyro, float dist, float dureeMission, float dureeTotal);

#endif
