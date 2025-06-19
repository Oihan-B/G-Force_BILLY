#include "billy.h"
#include "pins.h"

void scenario_1(float dist, float consigne_vitesse){

  char decision = suiviLigne();

  while(distanceAtteinte(dist)==0){

    decision = suiviLigne();

    if (decision == "A"){
      avancer(consigne_vitesse);
    }
    else if (decision == "G"){
      tournerGsoft(consigne_vitesse, 0.5);
    }
    else if (decision == "D"){
      tournerDsoft(consigne_vitesse, 0.5);
    }
    else{
      gyro(1);
      arreter();
    }   
  }
}
