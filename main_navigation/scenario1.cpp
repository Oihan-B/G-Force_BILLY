#include "billy.h"
#include "pins.h"

void scenario_1(int dist, float consigne_vitesse){

  char decision = suivi_lignes();

  while(distanceAtteinte(dist)==0){

    decision = suivi_lignes();

    if (decision == "A"){
      avancer(consigne_vitesse);
    }
    else if (decision == "G"){
      tournerG(consigne_vitesse);
    }
    else if (decision == "D"){
      tournerD(consigne_vitesse);
    }
    else{
      gyro(1);
      arreter();
      return 0;
    }   
  }
}
