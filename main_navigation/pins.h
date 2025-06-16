#ifndef PINS_H
#define PINS_H

// Moteurs

#define PWMMOTEURDROIT 23       // PWM
#define DIRECTIONMOTEURDROIT 21

#define PWMMOTEURGAUCHE 22       // PWM
#define DIRECTIONMOTEURGAUCHE 20


// Encodeurs

#define ENCODEURDROITA 25
#define ENCODEURDROITB 24

#define ENCODEURGAUCHEA 28
#define ENCODEURGAUCHEB 29

// Suiveur de ligne

#define S1 2
#define S2 3
#define S3 4
#define S4 5
#define S5 6
#define CLP 9

// Capteurs US

#define TRIGGER 33
#define CAPTEUR_CG 15
#define CAPTEUR_CD 27
#define CAPTEUR_AG 31
#define CAPTEUR_AD 30

// IHM

#define BOUTON_HAUT 0
#define BOUTON_BAS 1
#define BOUTON_CONF 7
#define BOUTON_RET 8

#define SDA_ECRAN 18
#define SCL_ECRAN 19

// GYROPHARE

#define GYROPHARE 26

// ESP32

#define ESP32_RX 17
#define ESP32_TX 16

#endif
