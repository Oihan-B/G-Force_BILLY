#include "billy.h"
#include "pins.h"

void scenario1(float dist, float consigne_vitesse){
  lancerMission(1, millis());
  Serial4.write("Bienvenue dans le scéanrio 1");
  Serial4.write("Je vais essayer de faire de mon mieux !");

  
  tournerAngleG(consigne_vitesse, 1, pi/2);
  delay(10000);

  /*

  distanceTotal   = 0;
  compteDroit     = 0;
  compteGauche    = 0;
  x = y = theta = 0;

  char decision = suiviLigne();
  float AG;
  float AD;

  while(distanceAtteinte(dist)==0){

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
    AG = 0;// lectureCapteurUltrason(CAPTEUR_AG, 3);
    AD = 0; //lectureCapteurUltrason(CAPTEUR_AD, 3);
  
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
        tournerG(consigne_vitesse, 0.7);
      }
      else if (decision == 'D'){
        gyro(0);
        tournerD(consigne_vitesse, 0.7);
      }
      else{
        gyro(1);
        arreter();
      }
    }
  }
  arreter();
  */
}
