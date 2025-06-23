#include "billy.h"
#include "pins.h"

void scenario2(float consigne_vitesse){
  lancerMission(2, millis());
  Serial4.write("Bienvenue dans le scéanrio 2");
  Serial4.write("Je pense sincèrement pouvoir y arriver !");

  distanceTotal   = 0;
  compteDroit     = 0;
  compteGauche    = 0;
  x = y = theta = 0;
  
  char decision = suiviLigne();

  while (decision != 'C'){

    if (Serial4.available()) {
      char c = Serial4.read();
      if(c == '{') {
        Serial4.write("Interruption du scénario 2 par l'administrateur");
        Serial4.write("Roger copy that, donne moi des ordres je m'éxecute !");
        controleManuel(consigne_vitesse);
        Serial4.write("Merci pour le dépannage, je reprend le contrôle !");
      }
    }

    decision = suiviLigne();
    
    if (AG != 0 || AD != 0){ // Interruption si obstacle
      contournerObstacle(consigne_vitesse);
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
        tournerGsoft(consigne_vitesse, 0.6);
      }
      else if (decision == 'D'){
        tournerDsoft(consigne_vitesse, 0.6);
      }
      else{
        gyro(1);
        arreter();
      }
    }
  }
}
