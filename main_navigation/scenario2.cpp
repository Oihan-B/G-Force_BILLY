#include "billy.h"
#include "pins.h"

void scenario_2(float consigne_vitesse){

  initCapteurUltrason();
  initSuiviLigne();
  
  char decision = suivi_lignes();

  while (decision != "C"){
    
    decision = suivi_lignes();
    
    if (lectureCapteurUltrason(CAPTEUR_AG) == 0 && lectureCapteurUltrason(CAPTEUR_AG) == 0){ // Interruption si obstacle
      gyro(1);
      contournerObstacle();
    }
  
    else{
      decision = suivi_lignes();
  
      if (decision == "C"){ 
        gyro(1);
        arreter();
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
        arreter();
        return 0;
      }
  
    }
  }
}

//C'est fini on a livré le colis il y'en a qu'un donc même pas besoin de revenir au spawn en vrai
