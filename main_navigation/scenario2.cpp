#include "billy.h"
#include "pins.h"

void scenario_2(float consigne_vitesse){
  
  char decision = suiviLigne();
  float AG;
  float AD;

  while (decision != "C"){
    
    decision = suiviLigne();

    AG = lectureCapteurUltrason(CAPTEUR_AG, 3);
    AD = lectureCapteurUltrason(CAPTEUR_AD, 3);
    
    if (AG == 0 || AD == 0){ // Interruption si obstacle
      gyro(1);
      contournerObstacle();
    }
  
    else{
      decision = suiviLigne();
  
      if (decision == 'C'){ 
        gyro(1);
        arreter();
      }
      else if (decision == 'A'){
        avancer(consigne_vitesse);
      }
      else if (decision == 'G'){
        tournerGsoft(consigne_vitesse, 0.5);
      }
      else if (decision == 'D'){
        tournerDsoft(consigne_vitesse, 0.5);
      }
      else{
        gyro(1);
        arreter();
      }
  
    }
  }
}

//C'est fini on a livré le colis il y'en a qu'un donc même pas besoin de revenir au spawn en vrai
