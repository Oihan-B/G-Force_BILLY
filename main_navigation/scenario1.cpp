#include "billy.h"
#include "pins.h"

void scenario_1(float dist, float consigne_vitesse){
  startMission(1, millis());
  Serial4.begin(115200);

  distanceTotal   = 0;
  compteDroit     = 0;
  compteGauche    = 0;
  x = y = theta = 0;

  char decision = suiviLigne();
  float AG;
  float AD;

  bool control = 0;

  while(distanceAtteinte(dist)==0){

    if (Serial4.available()) {
      char c = Serial4.read();
      if(c=="{"){
        control = 1;
      }else if(c=="}"){
        control = 0;
      }
    }

    if(control){
      gyro(1);
      //controleManuel(consigne_vitesse);
    }
    else{
      gyro(0);
      decision = suiviLigne();
      AG = lectureCapteurUltrason(CAPTEUR_AG, 3);
      AD = lectureCapteurUltrason(CAPTEUR_AD, 3);
    
      if (AG != 0 || AD != 0){ // Interruption si obstacle
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
  }

  arreter();
}
