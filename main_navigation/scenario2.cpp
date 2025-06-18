#include "billy.h"
#include "pins.h"

#define SPEED 150
#define TURN_SPEED 100

initCapteurUltrason();
initSuiviLigne();

char decision = suivi_lignes();

while (decision != "C"){

  if (lectureCapteurUltrason(CAPTEUR_AG) == 0 && lectureCapteurUltrason(CAPTEUR_AG) == 0){ // Interruption si obstacle
    gyro(1);
    contournerObstacle();
  }

  else{
    decision = suivi_lignes();

    if (decision == "C"){ 
      gyro(1);
      stopMoteurs();
      return 0;
    }
    else if (decision == "A"){
      avancer(SPEED);
    }
    else if (decision == "G"){
      tournerG(TURN_SPEED);
    }
    else if (decision == "D"){
      tournerD(TURN_SPEED);
    }
    else{
      gyro(1);
      stopMoteurs();
      return 0;
    }

  }
}

//C'est fini on a livré le colis il y'en a qu'un donc même pas besoin de revenir au spawn en vrai






