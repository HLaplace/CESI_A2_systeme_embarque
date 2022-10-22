////////////////////////////////////////////////////////////////////////////// lib //////////////////////////////////////////////////////////////////////////////
#include <SPI.h> // Communication modules SPI
#include <SD.h> // Communication carte SD
#include <LiquidCrystal.h> // Gestion ecran
#include <Wire.h>
#include <Adafruit_Sensor.h> // Gestion capteurs adafruit
#include <Adafruit_BME280.h> // Gestion capteur BME280 (temperature, pression, hygrometrie)
#include "RTClib.h" // Gestion horloge RTC
#include <ChainableLED.h> // Gestion led grove
#include <EEPROM.h> // Gestion EEPROM
#include <stdio.h> // Structure et liste chainée
#include <stdlib.h> // Structure et liste chainée
////////////////////////////////////////////////////////////////////////////// variable globale //////////////////////////////////////////////////////////////////////////////
typedef struct Mesure Mesure; // Creation d'une structure qui va recueillir differentes données capteurs
struct Mesure {
  short int temperature;
  short int pression;
  short int hygrometrie;
  short int luminosite;
  Mesure *suivant;
};

typedef struct Liste Liste; // Creation structure permettant de former la liste chainee
struct Liste {
  Mesure *premier;
};

#define PUSHPIN 2 // bouton rouge
#define PUSHPIN2 3 // bouton vert
  String tab[12] = {"LUMIN", "LUMIN_LOW", "LUMIN_HIGH", "TEMP_AIR", "MIN_TEMP_AIR", "MAX_TEMP_AIR", "HYGR", "HYGR_MINT", "HYGR_MAXT", "PRESSURE", "PRESSURE_MIN", "PRESSURE_MAX"};

#define SEALEVELPRESSURE_HPA (1013.25) // Etalonage du capteur
LiquidCrystal lcd = LiquidCrystal ( 8, 9, 10, 11, 12, 13 ) ; // Ecran
ChainableLED led(6, 7, 5); // Leds

RTC_Millis RTC; // RTC

short int CurrentMode = 1; //standard
short int PreviousMode = 1; // standard

static unsigned long previousMillis = 0; 
static unsigned long previousMillis_B = 0; 
unsigned long interval = 5000; // 180000000; // 30 min
static unsigned long temps = 0;

void buttoncheck(); // fonction interruption button rouge
void buttoncheck2(); // fonction interruption button vert

void Acquisition(); // fonction permettant d'acquerir les données capteurs
void Configuration(); // Mode Configuration

short int step = 0;

void Blink_main(int blink_count, unsigned long intervall, short int a1, short int b1, short int c1, short int a2, short int b2, short int c2); // Fonction permettant de faire clignoter les leds en cas d'erreurs
void write_SD(String file_name, String writing_text); // fonction permettant d'ecrire dans la carte SD
void display_screen(String texte); // fonction permettant d'afficher les donnees sur l'ecran
short int retour_luminosite(); // fonction permettant d'acquerir la luminosite
short int BME_value(int val); // fonction permettant d'acquerir la temperature, la pression et l'hygrometrie
String GPS_location(); // fonction permettant d'acquerir les coordonnées GPS

Liste *initListe(); // fonction permettant d'initialier la liste chainee
void ajouterdebut(Liste *liste, String coordonnees, String date, short int temperature, short int pression, short int hygrometrie, short int luminosite); // fonction permettant d'ajouter un element a la liste chainee
short int compter(Liste *liste); // fonction permettant de compter le nombre d'element de la liste chainee
short int moyenne(Liste *liste, int val); // fonction permettant d'effectuer la moyenne des donnees contenues dans les elements de la liste chainee

Liste *listMesure = initListe(); // creation de la liste chainee

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////// setup /////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup()
{
  Serial.begin(9600);

  RTC.begin(DateTime(__DATE__, __TIME__)); // Reglage de la RTC à l'heure actuel

  // Variables stockes dans l'EEPROM
  EEPROM.put(0, 1);
  EEPROM.put(1, 255);
  EEPROM.put(2, 768);
  EEPROM.put(3, 1);
  EEPROM.put(4, -10);
  EEPROM.put(5, 60);
  EEPROM.put(6, 1);
  EEPROM.put(7, 0);
  EEPROM.put(8, 50);
  EEPROM.put(9, 1);
  EEPROM.put(10, 850);
  EEPROM.put(11, 1080);
  
  pinMode(PUSHPIN, INPUT); // Initialisation bouton rouge
  pinMode(PUSHPIN2, INPUT); // Initialisation bouton vert

  attachInterrupt(digitalPinToInterrupt(PUSHPIN),buttoncheck,CHANGE); // definition du bouton rouge comme bouton d'interruption
  attachInterrupt(digitalPinToInterrupt(PUSHPIN2),buttoncheck2,CHANGE); // definition du bouton vert comme bouton d'interruption
  lcd.clear(); // suppression des entrees de l'ecran
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////// main //////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void loop()
{
  switch (CurrentMode) { 
  case 1: // Basculer en mode Standard
    led.setColorRGB(0, 0, 255, 0); // vert
    Acquisition(); // Acquisition des donnees
    break;

  case 2: // Basculer en mode Configuration
    Configuration(); 
    break;

  case 3:// Basculer en mode Maintenance
    led.setColorRGB(0, 255, 123, 0); // orange
    Acquisition(); // Acquisition des donnees
    break;

  case 4: // Basculer en mode Economique
    led.setColorRGB(0, 0, 0, 255); // bleu
    Acquisition(); // Acquisition des donnees
    break;
  }
}
////////////////////////////////////////////////////////////////////////////// Fonction d'interruption bouton rouge //////////////////////////////////////////////////////////////////////////////
void buttoncheck()
{
  unsigned long debutAppui = millis(); 
  if (temps == 0){ // Lorsque le bouton est presse
    temps = millis();
  }
  else {
    if ((millis() - temps)  < 10) {
      // Anti rebond
    }
    else {
      switch (CurrentMode) {
        case 1:
          if (debutAppui <= 10000){ // Passer de Standard a Configuration (uniquement valable les 10 première secondes apres le lancement
            CurrentMode = 2;
            PreviousMode = 1; // Recupérer et stocker le mode précédent
            temps = 0;
          }
          else { // Passer de mode Standard a mode Maintenance
            if (millis() - temps  > 5000){ // Bouton appuyer pendant 5 secondes
              CurrentMode = 3;
              PreviousMode = 1; // Recupérer et stocker le mode précédent
              temps = 0;
            }
            else{ // Simule un appui court
              temps = 0;
            }
          }
          temps = 0;
          break;
        case 3: // Passer de Maintenance au mode précédent (Standard ou Economique)
          if (millis() - temps  > 5000){ // Bouton appuyer pendant 5 secondes
            CurrentMode = PreviousMode;
            PreviousMode = 3; // Recupérer et stocker le mode précédent
            temps = 0;
          }
          else{ // Simule un appui court
            temps = 0;
          }
          break;
        case 4: // Passer de Economique a Standard
          if (millis() - temps  > 5000){ // Bouton appuyer pendant 5 secondes
            CurrentMode = 1;
            PreviousMode = 4; // Recupérer et stocker le mode précédent
            temps = 0;
          }
          else{ // Simule un appui court
            temps = 0;
          }
          break;     
      }
    }
  }
}
////////////////////////////////////////////////////////////////////////////// Fonction d'interruption bouton vert //////////////////////////////////////////////////////////////////////////////
void buttoncheck2()
{
  if (temps == 0){ // Lorsque le bouton est presse
    temps = millis();
  }
  else {
    if (millis() - temps  < 10) {
      // Anti rebond
    }
    else {
      switch (CurrentMode) {
        case 1:
          if (millis() - temps  > 5000){ // Bouton appuyer pendant 5 secondes
            CurrentMode = 4;
            PreviousMode = 1; // Recupérer et stocker le mode précédent
            temps = 0;
          }
          else{ // Simule un appui court
            temps = 0;
          }
          break;
        case 4:
          if (millis() - temps  > 5000){ // Bouton appuyer pendant 5 secondes
            CurrentMode = 3;
            PreviousMode = 4; // Recupérer et stocker le mode précédent
            temps = 0;
          }
          else{ // Simule un appui court
            temps = 0;
          }
          break;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////// mode configuration //////////////////////////////////////////////////////////////////////////////

void Configuration()
{
 led.setColorRGB(0, 255, 255, 0); // jaune 
  if(millis() - previousMillis <= interval){ // Boucle 30 min 
    Serial.println("Passage en mode config ");
  while (Serial.available()){
  String user_input = "";
  while (user_input == ""){
     user_input = Serial.readString();//recupere une chaine de caractere
  }
    user_input.trim();// suppression des espaces
    Serial.print(String("|") + user_input + String("|"));

for ( int i=0; i<11; i++){
  Serial.println(tab[i]);
  if (user_input == tab[i]){
    String new_var = "";
    Serial.print("entrez la nouvelle valeur de la variable : ");
    while (new_var == ""){
     new_var = Serial.readString(); // recupere une chaine de caractere
    }
    new_var.trim();
    EEPROM.write(i, new_var.toInt()); // ecrit la nouvelle valeur à stocker dans l'eeprom
    Serial.println("Variable rÃ©affecter avec succes -> " + String(new_var));
    }
    }if (user_input == "RESET"){
      Serial.print("reset des valeurs");                          
      setup();
      delay(50); 
    }else if (user_input == "VERSION"){
      Serial.print("Version 6.0 du programme");
      delay(50);
    }else{
      Serial.print("UNKNOW COMMAND");
    }
  }
  }
}
////////////////////////////////////////////////////////////////////////////// mode acquisition //////////////////////////////////////////////////////////////////////////////

void Acquisition(){
  short int temperature;
  short int pression;
  short int hygro;
  short int lumin;
  short int intervalMult = 1;

  if (CurrentMode == 3) {
    intervalMult = 2;
  }
  
  if ( ( millis() - previousMillis_B ) >= 5000 * intervalMult) {
    previousMillis_B = millis();

    DateTime rtc = RTC.now();
    String gps = GPS_location();
    String date = String(rtc.day())+ String("/") + String(rtc.month())+ String("/") + String(rtc.year())+ String(" ") + String(rtc.hour())+ String(":") + String(rtc.minute())+ String(":") + String(rtc.second());
    Serial.println(compter(listMesure));
    if (compter(listMesure) < 10){
      
      temperature = BME_value(1);
      pression = BME_value(2);
      hygro = BME_value(3);
      lumin = retour_luminosite();
  
      display_screen(date);
      display_screen(gps);
      display_screen(String("Temperature ") + temperature + String("C"));
      display_screen(String("Pression ") + pression + String("hPa")) ;
      display_screen(String("Hygrometrie ") + hygro + String("%")) ;
      display_screen(String("Luminosite ") + lumin + String("lux"));
  
      ajouterdebut(listMesure, temperature, pression, hygro, lumin);
    }
    else{
      temperature = moyenne(listMesure, 1);
      pression = moyenne(listMesure, 2);
      hygro = moyenne(listMesure, 3);
      lumin = moyenne(listMesure, 4);
      if (CurrentMode == 3){
        Serial.println("-----------------");
        Serial.println(gps);
        Serial.println(date);
        Serial.println(temperature);
        Serial.println(pression);
        Serial.println(hygro);
        Serial.println(lumin);
        Serial.println("-----------------");
      }

      if (CurrentMode == 1 or CurrentMode == 4){
        write_SD("data.txt", gps);
        write_SD("data.txt", date);
        write_SD("data.txt", String(temperature));
        write_SD("data.txt", String(pression));
        write_SD("data.txt", String(hygro));
        write_SD("data.txt", String(lumin));
      }
      supprimer(listMesure);
    }
  }
}

////////////////////////////////////////////////////////////////////////////// fonction affichage a l'ecran //////////////////////////////////////////////////////////////////////////////

void display_screen(String texte){
  short int index = texte.indexOf(' ');
  String texte1 = texte.substring(0,index);
  String texte2 = texte.substring(index+1,-1);
  lcd.begin( 16, 2 );
  lcd.setCursor(round((16-texte1.length())/2),0);
  lcd.print(texte1);
  lcd.setCursor(round((16-texte2.length())/2),1);
  lcd.print(texte2);
  delay(2000);
  lcd.clear();
}

////////////////////////////////////////////////////////////////////////////// fonction ecriture de données dans la carte SD //////////////////////////////////////////////////////////////////////////////

void write_SD(String file_name, String writing_text){
  File myFile;
  Serial.print("Initializing SD card...");
  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    Blink_main(2, 1000, 255, 0, 0, 255, 255, 255);
    // while (1);

    }
  Serial.println("initialization done.");
  myFile = SD.open(file_name, FILE_WRITE);
  if (myFile) {
    Serial.println("Writing to data.txt...");
    myFile.println(writing_text);
    myFile.close();
    Serial.println("done.");
  } 
  else {
    Serial.println("error opening " + file_name);
  }
}

////////////////////////////////////////////////////////////////////////////// fonction du capteur de luminosité //////////////////////////////////////////////////////////////////////////////

short int retour_luminosite(){
   short int erreur = 0;
   if (erreur == 1){
    Serial.println("erreur");
    Blink_main(1, 1000, 255, 0, 0, 255, 255, 0);
   }
   else{
    return analogRead(A2);
   }
 }

////////////////////////////////////////////////////////////////////////////// fonction du capteur BME //////////////////////////////////////////////////////////////////////////////

 short int BME_value(int val){
  Adafruit_BME280 bme;
  short int tab[3];
    if (!bme.begin(0x76)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    Blink_main(2, 1000, 255, 0, 0, 0, 255, 0);
    while (1);
  }
  tab[0] = bme.readTemperature();
  tab[1] = bme.readPressure() / 100.0F;
  tab[2] = bme.readHumidity();

  return tab[val - 1];
 }
////////////////////////////////////////////////////////////////////////////// fonction simulant l'appel au GPS //////////////////////////////////////////////////////////////////////////////

 String GPS_location(){
  short int erreur = 0;
  if (erreur == 1){
    Serial.println("Erreur acces gps");
    Blink_main(1, 1000, 255, 0, 0, 255, 255, 0);
    return "Erreur city";
    }
  else{
  return "Bordeaux";
  }
 }

////////////////////////////////////////////////////////////////////////////// fonction de clignotement simple //////////////////////////////////////////////////////////////////////////////

void Blink_main(int blink_count, unsigned long interval2, short int a1, short int b1, short int c1, short int a2, short int b2, short int c2){
  unsigned long currentMillis = millis();
  if(currentMillis - previousMillis >= interval2 and step == 0){
    previousMillis = currentMillis;
    led.setColorRGB(0, a1, b1, c1);
    step = 1;
  }
  if(currentMillis - previousMillis >= interval2 and step == 1){
    previousMillis = currentMillis;
    led.setColorRGB(0, a2, b2, c2);
    step = 2;
  }

    if(currentMillis - previousMillis >= interval2 and step == 2 and blink_count == 2){
    previousMillis = currentMillis;
    led.setColorRGB(0, a2, b2, c2);
    step = 0;
  }

}

////////////////////////////////////////////////////////////////////////////// Fonction d'initialisation de la liste chainée //////////////////////////////////////////////////////////////////////////////
Liste *initListe() {
  Liste *liste = (Liste*)malloc(sizeof(*liste));
  Mesure *element = (Mesure*)malloc(sizeof(*element));

  if (liste == NULL || element == NULL)
  {
      exit(EXIT_FAILURE);
  }

  element->temperature = 0;
  element->pression = 0;
  element->hygrometrie = 0;
  element->luminosite = 0;

  element->suivant = NULL;
  liste->premier = element;

  return liste;
}

////////////////////////////////////////////////////////////////////////////// Fonction permettant d'ajouter un element à la liste //////////////////////////////////////////////////////////////////////////////
void ajouterdebut(Liste *liste, short int temperature, short int pression, short int hygrometrie, short int luminosite) {
  /* Création nouvelle element*/
  Mesure *newData = (Mesure*)malloc(sizeof(*newData));
  
  newData->temperature = temperature;
  newData->pression = pression;
  newData->hygrometrie = hygrometrie;
  newData->luminosite = luminosite;

  /* Insertion de l'élément crée au début de la liste */
  newData->suivant = liste->premier;
  liste->premier = newData;
}

////////////////////////////////////////////////////////////////////////////// Fonction permettant de supprimer les elements de la liste //////////////////////////////////////////////////////////////////////////////
void supprimer(Liste *liste) {
  if(liste != NULL) {
    if(liste->premier == NULL) {
      liste = NULL;
    } else {
      Mesure* temp = liste->premier;
      while(temp->suivant->suivant != NULL)
        temp = temp->suivant;
      Mesure* lastNode = temp->suivant;
      temp->suivant = NULL;
      free(lastNode); 
    }
  }
}
////////////////////////////////////////////////////////////////////////////// Fonction permettant de compter le nombre d'element de la liste //////////////////////////////////////////////////////////////////////////////
short int compter(Liste *liste){
  short int i = 0;
  Mesure *x = liste->premier;
  while (x->suivant != NULL) {
    x = x->suivant;
    i++;
  }
  return i;
}
////////////////////////////////////////////////////////////////////////////// Fonction permettant de lire les données de chaque element de la liste //////////////////////////////////////////////////////////////////////////////

short int moyenne(Liste *liste, int val) {
  if (liste == NULL)
  {
      exit(EXIT_FAILURE);
  }
  short int tab[4] = {0,0,0,0};
  Mesure *x = liste->premier;
  while (x != NULL) {
    tab[0] += x->temperature;
    tab[1] += x->pression;
    tab[2] += x->hygrometrie;
    tab[3] += x->luminosite;
    x = x->suivant;
  }
  for (short int t=0; t<4; t++)
   {
   tab[t] = tab[t] / 10;
   }
   
  return tab[val - 1];
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
