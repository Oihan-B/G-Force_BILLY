#include <LiquidCrystal_I2C.h>
#ifndef BILLY_H
#define BILLY_H

// -----------------------------------------------------------------------------
// VARIABLES GLOBALES
// -----------------------------------------------------------------------------

#define SPEED 0.3               // Consigne de vitesse par défaut, utilisé par le PID pour ajuster le signal PWM des moteurs

#define ENTRAXE 320             // Écart entre les 2 roues
#define NB_TIC 1560             // Nombre de tics par tours de roues
#define D_ROUE 100              // Diametre roue
#define TIMERINTERVALE 20000    // Intervalle en ms de la fonction timer qui interromp le programme à chaque intervalle pour éxecuter du code prioritaire


//Définition des variables externes (de billy.cpp) afin qu'elles puissent être accessibles depuis tout les .cpp qui incluent billy.h

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
extern volatile double theta;          // radian 

extern float  marge;
extern double ancienConsigneDroit;
extern double ancienConsigneGauche;
extern double consigneDroit;
extern double consigneGauche;

extern float AG;
extern float AD;
extern float CG;
extern float CD;

extern float dureeTotal;
extern float dureeMission;
extern float debutMission;

extern double derniereLectureUltrason;
extern double tempsLectureUltrason;
extern double derniereMAJ;
extern double tempsMAJ;

extern int etatRobot;
extern int etatGyro;


// -----------------------------------------------------------------------------
// PROTOTYPES DES FONCTIONS
// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------
// Commandes Moteurs
// -----------------------------------------------------------------------------

void initMoteurs();                                           //Initialise les PINs relatives au contrôle du moteur (PWM et DIR) en OUTPUT
void avancerMoteurDroit(int pwm);                             //Fait avancer le moteur droit au signal pwm défini
void avancerMoteurGauche(int pwm);                            //Fait avancer le moteur gauche au signal pwm défini
void reculerMoteurDroit (int pwm);                            //Fait reculer le moteur droit au signal pwm défini
void reculerMoteurGauche (int pwm);                           //Fait avancer le moteur gauche au signal pwm défini
void stopMoteurs();                                           //Bloque les 2 moteurs, pwm nul
void setPwmEtDirectionMoteurs(int pwmGauche, int pwmDroit);   //Détermine la direction souhaité pour chaque moteur en fonction du signe des PWMs et utilise les fonctions correspondantes avancer/reculer
void avancer (float v);                                       //Fait avancer le robot (appel au PID pour équilibrer le pwm des fonctions avancer pour correspondre à la vitesse de consigne)
void reculer (float v);                                       //Fait reculer le robot (appel au PID pour équilibrer le pwm des fonctions reculer pour correspondre à la vitesse de consigne)
void tournerD (float v, float coeff);                         //Fait tourner à droite le robot (appel au PID pour équilibrer le pwm des fonctions avancer MG (100% vit) / reculer MD (p % * -vit) pour correspondre à la vitesse de consigne)
void tournerG (float v, float coeff);                         //Fait tourner à gauche le robot (appel au PID pour équilibrer le pwm des fonctions avancer MD (100% vit) / reculer MG (p % * -vit) pour correspondre à la vitesse de consigne)
void tournerDsoft(float v, float coeff);                      //Fait tourner à droite le robot par différentiel de vitesses, sens de rotation identique des 2 roues
void tournerGsoft(float v, float coeff);                      //Fait tourner à gauche le robot par différentiel de vitesses, sens de rotation identique des 2 roues
void arreter();                                               //Réinitialise les consignes de vitesses à 0 pour arrêter le robot
void tournerAngleD (float v, float coeff, float angle);       //Fait pivoter le robot d'un angle donné vers la droite
void tournerAngleG (float v, float coeff, float angle);       //Fait pivoter le robot d'un angle donné vers la gauche

// -----------------------------------------------------------------------------
// Fonctions Odometrie
// -----------------------------------------------------------------------------

void initEncodeurs();                 //Initialise les PINs relatives aux encodeurs, A et B avec un attachInterrupt pour faire appel aux fonctions compterDroit / compterGauche à chaque variation
void interruptionTimer();             //Fonction executée par le timer, calculs d'odométrie, lancement du PID, actualisation du site web...
void compterDroit();                  //Actualise la distance parcourue par le moteur droit en fontion des retours encodeurs (de combien la roue a tourné)
void compterGauche();                 //Actualise la distance parcourue par le moteur gauche en fontion des retours encodeurs (de combien la roue a tourné)
int distanceAtteinte(int dist);       //Compare une distance de consigne avec la distance parcourue de réference pour vérifier si un objectif a été atteint

// -----------------------------------------------------------------------------
// PID
// -----------------------------------------------------------------------------

void runPidMoteurs(float cmdG, float cmdD);    //Adapte le signal pwm de chaque moteur en fonction de sa consigne de vitesse (m/s) et de la vitesse actuelle déterminée par l'odométrie

// -----------------------------------------------------------------------------
// Detection Obstacles
// -----------------------------------------------------------------------------

void initCapteurUltrason();                                //Initialise les PINs relatives aux capteurs, un TRIGGER commun en OUTPUT et les ECHOs de chaque capteur en INPUT
float lectureCapteurUltrason(int capteur, int size);       //Renvoie la donnée lu par un capteur si 5 < detection < 40 sinon renvoie 0, on fait size lectures pour et conserve le min pour éviter un éventuel bruit dans la lecture
void contournerObstacle(float vit);                                 //Contourner l'obstacles une fois détectée en le longant en suivant ses côtés avec une marge de sécurité puis en reprenant la ligne

// -----------------------------------------------------------------------------
// Suivi Lignes
// -----------------------------------------------------------------------------

void initSuiviLigne();      //Initialise les PINs relatives au suivi de ligne, 5 capteurs en INPUT
char suiviLigne ();         //Renvoie un ordre d'une lettre en fonction du pattern des 5 capteurs, A pour avancer si la ligne est parfaitement centré, D s'il faut se décaler vers la droite pour récuperer la ligne,
                            //pareil pour G, C si on détecte un checkpoint, une étape de la mission indiqué par une ligne horizontale perpendiculaire à la ligne, S pour stop dans tout les autres cas => ligne perdue

// -----------------------------------------------------------------------------
// GYRO
// -----------------------------------------------------------------------------

void gyro (int etat);       //Allume (1) ou éteint (0) le gyrophare

// -----------------------------------------------------------------------------
// IHM
// -----------------------------------------------------------------------------

extern LiquidCrystal_I2C lcd;                                                                             //Déclaration de l'écran LCD
int boutonPresse();                                                                                       //Fonction qui renvoie -1 si aucun bouton pressé, sinon 0 pour haut, 1 pour bas, 2 pour confirmation et 3 pour retour
void afficherEcran(int duree, const char *txt1, const char *txt2, const char *txt3, const char *txt4);    //Affiche la saisie sur l'écran pendant duree millisecondes, 20 caractères max par lignes (4)
int confirmationCourrier();                                                                               //Confirmation de réeption, met en pause le robot et affiche un texte tant que l'utilisateur ne confirme pas

// -----------------------------------------------------------------------------
// SCENARIOS
// -----------------------------------------------------------------------------

void lancerMission(int s, float t);                        //Assignation des variables d'états et temporelles avant le lancement d'un scénario pour la supervision sur le site web
void scenario1(float dist, float consigne_vitesse);        //Scénario 1 : distance de consigne, tant qu'on peut suivre la ligne et qu'on a pas parcourue la distance définie, on continue
void scenario2(float consigne_vitesse);                    //Scénario 2 : d'un point A à un point B avec obstacles à éviter sur le parcours
void scenario3(float consigne_vitesse);                    //Scénario 3 : points de livraisons intermédiaires, avec retour au point de départ à la fin, pas d'obstacles

// -----------------------------------------------------------------------------
// SUPERVISION
// -----------------------------------------------------------------------------

void controleManuel(float vit);                                                                    //Fonction de contrôle manuel, priorise les ordres données par l'utilisateur depuis le site web tant que le controle reste actif
void actualiserSiteWeb(int etatRobot, float vitD, float vitG, float posX, float posY,              //Fonction d'actualisation qui transmet toutes les variables importantes à l'ESP32 pour permettre l'actualisation du site web
                        float CAPTEUR_CG, float CAPTEUR_AG, float CAPTEUR_AD, float CAPTEUR_CD, 
                        int etatGyro, float dist, float dureeMission, float dureeTotal);

#endif