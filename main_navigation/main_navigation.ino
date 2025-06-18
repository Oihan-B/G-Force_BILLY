#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "billy.h"
#include "pins.h"
 

LiquidCrystal_I2C lcd(0x27, 20, 4);
const int boutons[] = { BOUTON_HAUT, BOUTON_BAS, BOUTON_CONF, BOUTON_RET };
const int NB_BOUTONS = 4;

// -----------------------------------------------------------------------------
// États du menu
// -----------------------------------------------------------------------------

bool    enSousMenu   = false;
uint8_t selMenu      = 0;    // 0..3
uint8_t selSous      = 0;    // index dans le sous-menu
bool    etatParam[4] = { false, false, false, false }; // pour Scenario 2-3

// -----------------------------------------------------------------------------
// Définition du menu
// -----------------------------------------------------------------------------

#define NB_MENU 4
const char* menuPrincipal[NB_MENU] = {
  "Supervision BILLY",
  "Config vitesse",
  "Scenario 1",
  "Scenario 2-3"
};

// Nombre d'entrées dans chaque sous-menu
const uint8_t tailleSousMenu[NB_MENU] = {
  0,  // pas de sous-menu pour Supervision BILLY
  6,  // 0.2…0.7 m/s
  9,  // 3.00…5.00 m par 0.25
  5   // Scenario 2-3
};

// Contenu des sous-menus
const char* sousMenu1[] = {
  " 0.20 m/s"," 0.30 m/s"," 0.40 m/s",
  " 0.50 m/s"," 0.60 m/s"," 0.70 m/s"
};
const char* sousMenu2[] = {
  "  3.00 m","  3.25 m","  3.50 m","  3.75 m",
  "  4.00 m","  4.25 m","  4.50 m","  4.75 m","  5.00 m"
};
const char* sousMenu3[] = {
  "Arret point B",
  "Arret point C",
  "Retour point A",
  "Detect obstacles",
  "Valider mission"
};

// Pointeurs vers chaque sous-menu (nullptr = pas de sous-menu)
const char** tousSousMenus[NB_MENU] = {
  nullptr,
  sousMenu1,
  sousMenu2,
  sousMenu3
};

// -----------------------------------------------------------------------------
// Prototypes
// -----------------------------------------------------------------------------

int  bouton_presse();
void rafraichirMenu();
void boutonHaut();
void boutonBas();
void boutonValider();
void boutonRetour();
void wrapInc(uint8_t &v, uint8_t max);
void wrapDec(uint8_t &v, uint8_t max);

// Stubs des "actions" à remplacer par votre code
void actionSupervisionRobot();
void actionParametrerVitesse(float v);
void actionScenario1(float d);
void lancerMissionParam();

// -----------------------------------------------------------------------------
// Setup
// -----------------------------------------------------------------------------

void setup(){
  // boutons en pull-up
  for(int i = 0; i < NB_BOUTONS; i++){
    pinMode(boutons[i], INPUT_PULLUP);
  }
  // écran
  lcd.init();
  lcd.backlight();
  // afficher le menu initial
  rafraichirMenu();

  // inits BILLY
  initMoteurs();
  initEncodeurs();
  initSuiviLigne();
  initCapteurUltrason();
}

// -----------------------------------------------------------------------------
// Loop principal
// -----------------------------------------------------------------------------

void loop(){
  int b = bouton_presse();
  if(b < 0) return;

  if(b == BOUTON_HAUT)   boutonHaut();
  if(b == BOUTON_BAS)    boutonBas();
  if(b == BOUTON_CONF)   boutonValider();
  if(b == BOUTON_RET)    boutonRetour();
}

// -----------------------------------------------------------------------------
// Lecture d'un bouton (bloquant jusqu'au relâchement)
// -----------------------------------------------------------------------------

int bouton_presse(){
  for(int i = 0; i < NB_BOUTONS; i++){
    int pin = boutons[i];
    if(digitalRead(pin) == LOW){
      delay(20);
      while(digitalRead(pin) == LOW) ; // attente relâche
      delay(20);
      return pin;
    }
  }
  return -1;
}

// -----------------------------------------------------------------------------
// Rafraîchissement de l'affichage avec scroll + curseur mobile
// -----------------------------------------------------------------------------

void rafraichirMenu(){
  const int LINES = 4;
  lcd.clear();

  if(!enSousMenu){
    // menu principal
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
    // sous-menu
    int n = tailleSousMenu[selMenu];
    int off = selSous - (LINES - 1);
    if(off < 0)          off = 0;
    if(off > n - LINES)  off = n - LINES;

    for(int L = 0; L < LINES; L++){
      int idx = off + L;
      if(idx >= n) break;
      lcd.setCursor(0, L);
      lcd.print(idx == selSous ? "> " : "  ");

      // cas Scenario 2-3 avec checkbox pour les 4 premiers
      if(selMenu == 3 && idx < 4){
        lcd.print(tousSousMenus[3][idx]);
        lcd.print(' ');
        lcd.print(etatParam[idx] ? '1' : '0');
      }
      else {
        lcd.print(tousSousMenus[selMenu][idx]);
      }
    }
  }
}

// -----------------------------------------------------------------------------
// Navigation haut/bas
// -----------------------------------------------------------------------------

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
void wrapInc(uint8_t &v, uint8_t max){
  v++;
  if(v >= max) v = 0;
}
void wrapDec(uint8_t &v, uint8_t max){
  if(v == 0)  v = max - 1;
  else        v--;
}

void boutonValider(){
  if(!enSousMenu){
    // si sous-menu existant, on y entre
    if(tailleSousMenu[selMenu] > 0){
      enSousMenu = true;
      selSous    = 0;
      rafraichirMenu();
    }
    else {
      // action directe menu principal
      if(selMenu == 0){
        actionSupervisionRobot();
        lcd.clear();
        lcd.setCursor(0,0); lcd.print("Supervision en");
        lcd.setCursor(0,1); lcd.print("cours...");
        delay(1000);
      }
      rafraichirMenu();
    }
  }
  else {
    // dans un sous-menu
    switch(selMenu){
      case 1: { // Config vitesse
        float v = 0.2f + 0.1f * selSous;
        actionParametrerVitesse(v);
        lcd.clear();
        lcd.setCursor(0,0); lcd.print("Vitesse reglee");
        lcd.setCursor(0,1);
        lcd.print(v,2); lcd.print(" m/s");
        delay(1000);
        enSousMenu = false;
        break;
      }
      case 2: { // Scenario 1
        float d = 3.0f + 0.25f * selSous;
        actionScenario1(d);
        lcd.clear();
        lcd.setCursor(0,0); lcd.print("Scenario 1:");
        lcd.setCursor(0,1);
        lcd.print(d,2); lcd.print(" m en cours");
        delay(1000);
        enSousMenu = false;
        break;
      }
      case 3:  // Scenario 2-3
        if(selSous < 4){
          etatParam[selSous] = !etatParam[selSous];
        }
        else {
          // Valider mission
          lcd.clear();
          lcd.setCursor(0,0); lcd.print("Lancement mission");
          delay(500);
          lancerMissionParam();
          enSousMenu = false;
        }
        break;
    }
    rafraichirMenu();
  }
}

void boutonRetour(){
  if(enSousMenu){
    enSousMenu = false;
    rafraichirMenu();
  }
}

void actionSupervisionRobot(){
  // code de supervision BILLY
}
void actionParametrerVitesse(float v){
  // stocker la consigne de vitesse
}
void actionScenario1(float d){
  // scenario 1 distance = d mètres
}
void lancerMissionParam(){
  // exécution mission Scenario 2-3
}
