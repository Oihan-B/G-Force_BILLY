#include "billy.h"
#include "pins.h"

void scenario_1(float dist, float consigne_vitesse){

  gyro(1);

  char decision = suiviLigne();

  while(distanceAtteinte(dist)==0){

    decision = suiviLigne();

    if (decision == 'A'){
      avancer(-consigne_vitesse);
    }
    else if (decision == 'G'){
      tournerG(consigne_vitesse, 0.75);
    }
    else if (decision == 'D'){
      tournerD(consigne_vitesse, 0.75);
    }
    else{
      gyro(1);
      arreter();
    }   
  }

  arreter();
}
