#include "billy.h"
#include "pins.h"

void scenario1(float dist, float consigne_vitesse){
  lancerMission(1, millis());
  envoyerLogs("\nBienvenue dans le scéanrio 1");
  envoyerLogs("\nJe vais essayer de faire de mon mieux !");
/*
  while (1) {
    delay(100);
    // lecture des quatre capteurs à ultrasons

    // affichage
    Serial.print("AG : "); Serial.println(AG);
    Serial.print("AD : "); Serial.println(AD);
    Serial.print("CG : "); Serial.println(CG);
    Serial.print("CD : "); Serial.println(CD);

    // optionnel : séparation visuelle
    Serial.println("----------------");
  }

  */
  

  distanceTotal   = 0;
  compteDroit     = 0;
  compteGauche    = 0;
  x = y = theta = 0;

  char decision = suiviLigne();

  while(distanceAtteinte(dist)==0){

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
        tournerG(consigne_vitesse, 0.6);
      }
      else if (decision == 'D'){
        gyro(0);
        tournerD(consigne_vitesse, 0.6);
      }
      else if(decision == 'S'){
        delay(200);
        decision = suiviLigne();
        if(decision == 'S'){
          gyro(1);
          arreter();
        }
      }
      else{
        gyro(1);
        arreter();
      }
    }
  }
  
  arreter();
  afficherEcran(10000, "J'ai fini", "les 3 metres" , NULL, NULL);
  
}
