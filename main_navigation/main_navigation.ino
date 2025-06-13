#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "billy.h"
#include "pins.h"
 

LiquidCrystal_I2C lcd(0x27, 20, 4);
const int boutons[] = {BOUTON_HAUT, BOUTON_BAS, BOUTON_CONF, BOUTON_RET};  
const int NB_BOUTONS = 4;

// -----------------------------------------------------------------------------
// Definitions des menus 
// -----------------------------------------------------------------------------

bool    enSousMenu   = false;
uint8_t selMenu      = 0;      // 0..3
uint8_t selSous      = 0;      // index dans le sous-menu
bool    etatParam[4] = {0,0,0,0};

const int NB_MENU = 4;
const char* menuPrincipal[NB_MENU] = {
  "Param mission",
  "Test suivi lignes",
  "Modes vitesses",
  "Calibration"
};
const uint8_t tailleSousMenu[NB_MENU] = { 5, 9, 3, 1 };
 
const char* sousMenu0[] = {
  "Arret point B",
  "Arret point C",
  "Retour point A",
  "Detect obstacles",
  "Valider mission"
};
const char* sousMenu1[] = {
  "1 m","2 m","3 m","5 m","7 m","10 m","15 m","20 m","Infini"
};
const char* sousMenu2[] = {
  "Eco","Normal","Sport"
};
const char* sousMenu3[] = {
  "Lancer calib"
};
 
const char** tousSousMenus[NB_MENU] = {
  sousMenu0, sousMenu1, sousMenu2, sousMenu3
};
 
// -----------------------------------------------------------------------------
// Prototypes Fonctions
// -----------------------------------------------------------------------------

int  bouton_presse();
void rafraichirMenu();
void boutonHaut();
void boutonBas();
void boutonValider();
void boutonRetour();
 
void actionTest1()     {}
void actionTest2()     {}
void actionTest3()     {}
void actionTest5()     {}
void actionTest7()     {}
void actionTest10()    {}
void actionTest15()    {}
void actionTest20()    {}
void actionTestInfini(){}
void actionModeEco()    {}
void actionModeNormal() {}
void actionModeSport()  {}
void actionCalibration(){}
 
void (*actionSousMenu[NB_MENU][9])() = {
  { }, // menu 0 geré en dur
  { actionTest1, actionTest2, actionTest3, actionTest5,
    actionTest7, actionTest10, actionTest15, actionTest20, actionTestInfini },
  { actionModeEco, actionModeNormal, actionModeSport },
  { actionCalibration }
};


void setup(){
  for(int i=0; i < NB_BOUTONS; i++) pinMode(boutons[i], INPUT_PULLUP);
  lcd.init(); lcd.backlight();
 
  lcd.clear();
  lcd.setCursor(0,0); lcd.print("Bonjour, je suis");
  lcd.setCursor(0,1); lcd.print("BILLY votre robot");
  lcd.setCursor(0,2); lcd.print("livreur de courrier");
  lcd.setCursor(0,3); lcd.print("autonome en Estia 1");
  delay(3000);
 
  rafraichirMenu();
}
 

void loop(){
  int b = bouton_presse();
  if(b>0){
    switch(b){
      case 1: boutonHaut();    break;
      case 2: boutonBas();     break;
      case 3: boutonValider(); break;
      case 4: boutonRetour();  break;
    }
    delay(200);
    while(bouton_presse()!=0);
    delay(100);
  }
}


// -----------------------------------------------------------------------------
// Lecture des boutons (1 à 4 ou 0 indetermine)
// -----------------------------------------------------------------------------

int bouton_presse(){
  int trouve=0,ret=0;
  for(int i=0;i<NB_BOUTONS;i++){
    if(digitalRead(boutons[i])==LOW){
      if(trouve) return 0;
      ret = i+1; trouve=1;
    }
  }
  return ret;
}
 
// -----------------------------------------------------------------------------
// Affichage du menu ou sous-menu en « fenêtre » de 4 lignes
// -----------------------------------------------------------------------------

void rafraichirMenu(){
  lcd.clear();
 
  if(!enSousMenu){
    // Menu principal (4 lignes fixes)
    uint8_t n = NB_MENU;
    int off = selMenu<=1    ? 0 :
              selMenu>=n-2  ? n-4 :
              selMenu-1;
    for(uint8_t L=0; L<4; L++){
      uint8_t idx = (off+L) % n;
      lcd.setCursor(0,L);
      lcd.print(idx==selMenu?"> ":"  ");
      lcd.print(menuPrincipal[idx]);
    }
  }
  else {
    uint8_t n = tailleSousMenu[selMenu];
    if(n <= 4){
      // Affichage direct des n lignes (reste vide si L>=n)
      for(uint8_t L=0; L<4; L++){
        lcd.setCursor(0,L);
        if(L<n){
          uint8_t idx = L;
          lcd.print(idx==selSous?"> ":"  ");
          if(selMenu==0 && idx<4){
            // cases a cocher
            lcd.print(sousMenu0[idx]);
            lcd.print(" ");
            lcd.print(etatParam[idx]?'1':'0');
          } else {
            lcd.print(tousSousMenus[selMenu][idx]);
          }
        }
      }
    }
    else {
      // Défilement par fenêtre de 4 lignes
      int off = selSous<=1      ? 0 :
                selSous>=n-2    ? n-4 :
                selSous-1;
      for(uint8_t L=0; L<4; L++){
        uint8_t idx = (off+L) % n;
        lcd.setCursor(0,L);
        lcd.print(idx==selSous?"> ":"  ");
        if(selMenu==0 && idx<4){
          // cases a cocher aussi ici
          lcd.print(sousMenu0[idx]);
          lcd.print(" ");
          lcd.print(etatParam[idx]?'1':'0');
        }
        else {
          lcd.print(tousSousMenus[selMenu][idx]);
        }
      }
    }
  }
}
 
// -----------------------------------------------------------------------------
// Wrappers cycliques
// -----------------------------------------------------------------------------

static void wrapInc(uint8_t &v, uint8_t m){ v = (v+1) % m; }
static void wrapDec(uint8_t &v, uint8_t m){ v = (v+m-1) % m; }
 
// -----------------------------------------------------------------------------
// Gestion des boutons
// -----------------------------------------------------------------------------

void boutonHaut(){
  if(!enSousMenu)       wrapDec(selMenu,NB_MENU);
  else                  wrapDec(selSous,tailleSousMenu[selMenu]);
  rafraichirMenu();
}
 
void boutonBas(){
  if(!enSousMenu)       wrapInc(selMenu,NB_MENU);
  else                  wrapInc(selSous,tailleSousMenu[selMenu]);
  rafraichirMenu();
}
 
void boutonValider(){
  if(!enSousMenu){
    // Entree en sous-menu
    enSousMenu = true;
    selSous = 0;
    rafraichirMenu();
  }
  else if(selMenu==0){
    // Param mission : toggle ou bilan
    if(selSous<4){
      etatParam[selSous] = !etatParam[selSous];
      rafraichirMenu();
    }
    else {
      // Bilan
      lcd.clear();
      lcd.setCursor(0,0); lcd.print("BILAN MISSION");
      lcd.setCursor(0,1); lcd.print("Arret B:"); lcd.print(etatParam[0]?'1':'0');
      lcd.setCursor(0,2); lcd.print("Arret C:"); lcd.print(etatParam[1]?'1':'0');
      lcd.setCursor(0,3); lcd.print("Retour A:"); lcd.print(etatParam[2]?'1':'0');
      delay(500);
      while(bouton_presse()==0);
      rafraichirMenu();
    }
  }
  else {
    // Autres sous-menus
    (*actionSousMenu[selMenu][selSous])();
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(tousSousMenus[selMenu][selSous]);
    lcd.setCursor(0,1);
    lcd.print("execut ok");
    delay(1000);
    rafraichirMenu();
  }
}
 
void boutonRetour(){
  if(enSousMenu){
    enSousMenu = false;
    rafraichirMenu();
  }
}