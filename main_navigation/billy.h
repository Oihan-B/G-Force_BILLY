#ifndef BILLY_H
#define BILLY_H

#include <stdio.h>
#include <stdlib.h>


int calibration_suivi_lignes();


int suivre_ligne ();
/*Boucle de suivi de ligne avec appel à la fonction de monitoring toutes les x secondes
et vérificationdes inputs en permanence pour une pause d'urgence
conditions d'arrêt : obstacle detectee (return 0) ou checkpoint detectee (ligne large)(return 1)
en fonction de l'historique de la mission en cours et du parametrage, le robot s'arrêtera ou non, s'il
s'arrete il attend la confirmation du l'utilisateur pour qu'il est le temps de réceptionner le colis avant de redémarrer*/

void avancerMoteurDroit(uint8_t pwm);
void avancerMoteurGauche(uint8_t pwm);
void reculerMoteurDroit (uint8_t pwm);
void reculerMoteurGauche (uint8_t pwm);
void stopMoteurs();
void avancer (uint8_t pwm);
void reculer (uint8_t pwm);
void tournerD (uint8_t pwm);
void tournerG (uint8_t pwm);

void compterDroit();
void compterGauche();


int eviter_obstacles ();



void monitoring ();
/*collecte de toutes les données relatives au robot et à la mission et actualisation du site web de supervision*/

#endif
