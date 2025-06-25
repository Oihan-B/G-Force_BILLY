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
        gyro(1);
        afficherEcran(0, "Bon, je vais", "essayer de", "contourner l'obstacle", "A + dans l'bus");
        contournerObstacle(consigne_vitesse);
        gyro(0);
        afficherEcran(2000, "Je suppose que", NULL, "j'ai reussi", NULL);
        afficherEcran(0, "Scenario 2...", NULL, NULL, NULL);
        obs = 1;
      }
    }
  
    else{
      decision = suiviLigne();
  
      if (decision == 'C'){ 
        gyro(1);
        arreter();
        confirmationCourrier();
        afficherEcran(2000, "Scenario 3", NULL, "terminee", NULL);
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
      else if(decision == 'S'){
        delay(500);
        decision = suiviLigne();
        if(decision == 'S'){
          gyro(1);
          arreter();
          afficherEcran(2000, "Oups, j'ai perdu", NULL, "la ligne", NULL);
        }
      }
      else{
        gyro(1);
        arreter();
      }
    }
  }
}