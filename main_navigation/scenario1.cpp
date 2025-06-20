#include "billy.h"
#include "pins.h"

void scenario_1(float dist, float consigne_vitesse){
  debutMission(1, millis());

  distanceTotal   = 0;
  compteDroit     = 0;
  compteGauche    = 0;
  x = y = theta = 0;

  char decision = suiviLigne();

  while(distanceAtteinte(dist)==0){

    decision = suiviLigne();
    AG = lectureCapteurUltrason(CAPTEUR_AG, 3);
    AD = lectureCapteurUltrason(CAPTEUR_AD, 3);

    if (AG == 0 || AD == 0){ // Interruption si obstacle
      gyro(1);
      arreter();
    }
    else{
      if (decision == 'A'){
        gyro(0);
        avancer(-consigne_vitesse);
      }
      else if (decision == 'G'){
        gyro(0);
        tournerG(consigne_vitesse, 0.75);
      }
      else if (decision == 'D'){
        gyro(0);
        tournerD(consigne_vitesse, 0.75);
      }
      else{
        gyro(1);
        arreter();
      }
    }
  }

  arreter();
}
