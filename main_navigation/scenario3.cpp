#include "billy.h"
#include "pins.h"

void scenario_3(float consigne_vitesse){
  startMission(3, millis());

  distanceTotal   = 0;
  compteDroit     = 0;
  compteGauche    = 0;
  x = y = theta = 0;

  char decision = suiviLigne();
  float AG;
  float AD;

  int control = 0;

  int checkpoint = 0;

  while(checkpoint<=2){
    if(control){
      //controleManuel(consigne_vitesse);
    }
    else{
      
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
          avancer(consigne_vitesse);
        }
        else if (decision == 'G'){
          gyro(0);
          tournerG(consigne_vitesse, 0.75);
        }
        else if (decision == 'D'){
          gyro(0);
          tournerD(consigne_vitesse, 0.75);
        }
        else if (decision == 'C'){
          gyro(0);
          arreter();
          delay(5000);
          checkpoint++;
          avancer(consigne_vitesse);
          delay(250);
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
