/*
#include <ESP_Mail_Client.h>
#include <Wifi.h>

const char* WIFI_SSID     = "Nothing_1";
const char* WIFI_PASSWORD = "1415926535";

#define SMTP_HOST        "smtp.gmx.com"
#define SMTP_PORT        esp_mail_smtp_port_587
#define AUTHOR_EMAIL     "g-force.billy@gmx.fr"
#define AUTHOR_PASSWORD  "Rodolphe64!"

SMTPSession    smtp;
Session_Config config;
SMTP_Message   message;

void smtpCallback(SMTP_Status status){
  Serial.println(status.info());
}

void setup(){
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED){
    delay(200);
    Serial.print('.');
  }
 
  Serial.println("\nWi-Fi connecté");
  Serial.println(WiFi.localIP());

  MailClient.networkReconnect(true);

  smtp.debug(1);
  smtp.callback(smtpCallback);

  config.server.host_name  = SMTP_HOST;
  config.server.port       = SMTP_PORT;
  config.login.email       = AUTHOR_EMAIL;
  config.login.password    = AUTHOR_PASSWORD;
  config.login.user_domain = F("127.0.0.1");

  config.time.ntp_server       = F("pool.ntp.org,time.nist.gov");
  config.time.gmt_offset       = 1;
  config.time.day_light_offset = 1;

  if (!smtp.connect(&config)){
    Serial.println("Échec connexion SMTP : " + smtp.errorReason());
    return;
  }

  message.sender.name = F("BILLY");
  message.sender.email = AUTHOR_EMAIL;
  message.subject = F("Livraison Courrier ESTIA 1");
  message.addRecipient(F("Flavien"), F("flavien.carrette@etu.estia.fr"));
  message.addRecipient(F("Unai"), F("unai.heguy--tellechea@etu.estia.fr"));
  message.addRecipient(F("Lucien"), F("lucien.soula@etu.estia.fr"));
  message.addRecipient(F("Angel"), F("angel.ngoko-zenguet@etu.estia.fr"));
  message.addRecipient(F("Taoues"), F("taoues.saci@etu.estia.fr"));
  message.addRecipient(F("Oihan"), F("oihan.barreneche@etu.estia.fr"));
  message.text.content = F(
  "Toc Toc Toc, c'est BILLY, je suis en face de la porte, ouvrez moi svp j'ai votre colis ! Le code du cadenas est 587.\n"
  "Si vous n'êtes pas là d'ici 5 minutes je jure que j'me barre, il faudra venir le chercher à l'accueil !\n"
  "\n\n"
  "Cordialement,\n"
  "La Team G-Force");

  message.text.transfer_encoding = "base64";
  message.text.charSet = F("utf-8");

  if (!MailClient.sendMail(&smtp, &message))
    Serial.println("Erreur envoi : " + smtp.errorReason());
  else
    Serial.println("E-mail envoyé !");
  smtp.closeSession();
}

void loop(){
  // Le mail n'est envoye qu'une fois au demarrage pour l'instant
}
*/