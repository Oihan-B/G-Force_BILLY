#include "billy.h"
#include "pins.h"

void scenario2(float consigne_vitesse){
  lancerMission(2, millis());
  envoyerLogs("\nBienvenue dans le scéanrio 2");
  envoyerLogs("\nJe pense sincèrement pouvoir y arriver !");

  distanceTotal   = 0;
  compteDroit     = 0;
  compteGauche    = 0;
  x = y = theta = 0;
  int obs = 0;
  
  char decision = suiviLigne();

  while (decision != 'C'){

    if (Serial4.available()) {
      char c = Serial4.read();
      if(c == '{') {
        envoyerLogs("\nInterruption du scénario 2 par l'administrateur");
        envoyerLogs("\nRoger copy that, donne moi des ordres je m'éxecute !");
        controleManuel(consigne_vitesse);
        envoyerLogs("\nMerci pour le dépannage, je reprend le contrôle !");
      }
    }

    decision = suiviLigne();
    
    if (((AG != 0 && AG <= 30) || (AD != 0 && AD <= 30)) && obs == 0){ // Interruption si obstacle
      delay(80);
      if ((AG != 0 && AG <= 30) || (AD != 0 && AD <= 30)){
        contournerObstacle(consigne_vitesse);
        obs = 1;
      }
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
