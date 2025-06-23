#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "billy.h"
#include "pins.h"

const int boutons[] = { BOUTON_HAUT, BOUTON_BAS, BOUTON_CONF, BOUTON_RET };
const int NB_BOUTONS = 4;

LiquidCrystal_I2C lcd(0x27, 20, 4);

bool    enSousMenu = false;
uint8_t selMenu    = 0;    // 0..4
uint8_t selSous    = 0;    // pour Config vitesse et Scenario 1

float vitesse = 0.25;
float distance = 1000;

// -----------------------------------------------------------------------------
// Définition du menu principal et des sous-menus
// -----------------------------------------------------------------------------

#define NB_MENU 5
const char* menuPrincipal[NB_MENU] = {
  "Supervision BILLY",
  "Config vitesse",
  "Scenario 1",
  "Scenario 2",
  "Scenario 3"
};

// Nombre d'entrées dans chaque sous-menu (0 = action directe)

const uint8_t tailleSousMenu[NB_MENU] = {
  0,  // Supervision BILLY
  8,  // Config vitesse
  9,  // Scenario 1
  0,  // Scenario 2  
  0   // Scenario 3   
};

const char* sousMenuVitesse[] = {
  "  0.15 m/s","  0.20 m/s","  0.25 m/s","  0.30 m/s",
  "  0.35 m/s","  0.40 m/s","  0.45 m/s","  0.50 m/s"
};
const char* sousMenuDistance[] = {
  "  3.00 m","  3.25 m","  3.50 m","  3.75 m",
  "  4.00 m","  4.25 m","  4.50 m","  4.75 m","  5.00 m"
};

// Pointeurs vers chaque sous-menu (nullptr = pas de sous-menu)

const char** tousSousMenus[NB_MENU] = {
  nullptr,
  sousMenuVitesse,
  sousMenuDistance,
  nullptr,
  nullptr
};

// -----------------------------------------------------------------------------
// Fonctions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Rafraîchissement de l'affichage
// -----------------------------------------------------------------------------

void rafraichirMenu(){
  const int LINES = 4;
  lcd.clear();

  if(!enSousMenu){
    // Menu principal
    int n = NB_MENU;
    int off = selMenu - (LINES - 1);
    if(off < 0)          off = 0;
    if(off > n - LINES)  off = n - LINES;
    for(int L = 0; L < LINES; L++){
      int idx = off + L;
      if(idx >= n) break;
      lcd.setCursor(0, L);
      lcd.print(idx == selMenu ? "> " : "  ");
      lcd.print(menuPrincipal[idx]);
    }
  }

  else {
    // Sous-menu (Config vitesse ou Scenario 1)
    int n = tailleSousMenu[selMenu];
    int off = selSous - (LINES - 1);
    if(off < 0)          off = 0;
    if(off > n - LINES)  off = n - LINES;
    for(int L = 0; L < LINES; L++){
      int idx = off + L;
      if(idx >= n) break;
      lcd.setCursor(0, L);
      lcd.print(idx == selSous ? "> " : "  ");
      lcd.print(tousSousMenus[selMenu][idx]);
    }
  }
}

// -----------------------------------------------------------------------------
// Navigation haut/bas
// -----------------------------------------------------------------------------

void wrapInc(uint8_t &v, uint8_t max){
  if(++v >= max) v = 0;
}
void wrapDec(uint8_t &v, uint8_t max){
  if(v == 0) v = max - 1;
  else       v--;
}

void boutonHaut(){
  if(!enSousMenu)      wrapDec(selMenu, NB_MENU);
  else                 wrapDec(selSous, tailleSousMenu[selMenu]);
  rafraichirMenu();
}
void boutonBas(){
  if(!enSousMenu)      wrapInc(selMenu, NB_MENU);
  else                 wrapInc(selSous, tailleSousMenu[selMenu]);
  rafraichirMenu();
}


// -----------------------------------------------------------------------------
// Bouton RETOUR
// -----------------------------------------------------------------------------

void boutonRetour(){
  if(enSousMenu){
    enSousMenu = false;
    rafraichirMenu();
  }
}

// -----------------------------------------------------------------------------
// Bouton CONFIRMATION
// -----------------------------------------------------------------------------

void boutonValider(){
  if(!enSousMenu){
    if(tailleSousMenu[selMenu] > 0){
      // Entrer dans le sous-menu Config vitesse ou Scenario 1
      enSousMenu = true;
      selSous    = 0;
    }
    else {
      // Actions directes
      lcd.clear();
      lcd.setCursor(0,0);
      switch(selMenu){
        case 0:
          lcd.print("Supervision...");
          while (1) {
            controleManuel(vitesse);
          }
        case 3:
          lcd.print("Scenario 2...");
          delay(2000);
          scenario2(vitesse);
          break;
        case 4:
          lcd.print("Scenario 3...");
          delay(2000);
          scenario3(vitesse);
          break;
      }
      delay(250);
    }
  }
  else {
    // Validation dans un sous-menu
    if(selMenu == 1){
      vitesse = 0.15f + 0.05f * selSous;
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Vitesse: ");
      lcd.print(vitesse,2);
      lcd.print(" m/s");
      delay(1000);
    }
    else if(selMenu == 2){
      distance = (3.0f + 0.25f * selSous) * 1000;
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Scenario 1:");
      lcd.setCursor(0,1);
      lcd.print(distance/1000,2);
      lcd.print(" m en cours");
      delay(2000);
      scenario1(distance, vitesse);
    }
    enSousMenu = false;
  }
  rafraichirMenu();
}

// -----------------------------------------------------------------------------
// Setup
// -----------------------------------------------------------------------------

void setup(){
  for(int i = 0; i < NB_BOUTONS; i++){
    pinMode(boutons[i], INPUT_PULLUP);
  }
  
  lcd.init();
  lcd.backlight();
  rafraichirMenu();
  Serial.begin(115200);
  Serial4.begin(115200);

  // Inits BILLY
  initMoteurs();
  initEncodeurs();
  initSuiviLigne();
  initCapteurUltrason();
  pinMode(GYROPHARE, OUTPUT);
  digitalWrite(GYROPHARE, LOW);
}

// -----------------------------------------------------------------------------
// Loop 
// -----------------------------------------------------------------------------

void loop(){
  int b = boutonPresse();
  if(b < 0) return;

  if(b == BOUTON_HAUT)   boutonHaut();
  if(b == BOUTON_BAS)    boutonBas();
  if(b == BOUTON_CONF)   boutonValider();
  if(b == BOUTON_RET)    boutonRetour(); 
}