#include "billy.h"
#include "pins.h"

void scenario_1(int dist){

  initSuiviLigne();
  distanceTotal = 0;
  char decision = suivi_lignes();

  while(distanceTotal < dist){

    while (decision != "C"){
      decision = suivi_lignes();

      if (decision == "C"){ 
        gyro(1);
        stopMoteurs();
        return 0;
      }
      else if (decision == "A"){
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
        stopMoteurs();
        return 0;
      }   
    }
  }
}

