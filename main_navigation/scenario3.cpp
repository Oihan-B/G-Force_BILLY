#include "billy.h"
#include "pins.h"

void scenario3(float consigne_vitesse){
  lancerMission(3, millis());
 envoyerLogs("\nBienvenue dans le scéanrio 3");
 envoyerLogs("\nAttachez vos ceintures les colis, c'est partiiiii");

  distanceTotal   = 0;
  compteDroit     = 0;
  compteGauche    = 0;
  x = y = theta = 0;

  char decision = suiviLigne();

  int checkpoint = 0;

  while(checkpoint <= 2){

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
  
    if ((AG != 0 && AG <= 40) || (AD != 0 && AD <= 40)){ // Interruption si obstacle
      delay(100);
      if ((AG != 0 && AG <= 40) || (AD != 0 && AD <= 40)){
        arreter();
        gyro(1);
        afficherEcran(2000, "Oups, j'ai rencontre", NULL, "un obstacle", NULL);
      }
    }
    else{
      if (decision == 'A'){
        gyro(0);
        avancer(consigne_vitesse);
      }
      else if (decision == 'G'){
        gyro(0);
        tournerG(consigne_vitesse, 0.6);
      }
      else if (decision == 'D'){
        gyro(0);
        tournerD(consigne_vitesse, 0.6);
      }
      else if (decision == 'C'){
        gyro(0);
        arreter();
        confirmationCourrier();
        checkpoint++;
        snprintf(buf, sizeof(buf), "Checkpoint %d/3", checkpoint);
        afficherEcran(2000, "Scenario 3 ...", NULL, buf, NULL);
        if(checkpoint!=2){
          avancerDist(consigne_vitesse, 75);
        }
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
  arreter();
}