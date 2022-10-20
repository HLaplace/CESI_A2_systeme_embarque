////////////////////////////////////////////////////////////////////////////// lib //////////////////////////////////////////////////////////////////////////////
#include <SPI.h>
#include <SD.h>
#include <LiquidCrystal.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "Arduino.h"
#include "uRTCLib.h"
#include <ChainableLED.h>
#include <EEPROM.h>
#include <stdio.h>
#include <stdlib.h>
////////////////////////////////////////////////////////////////////////////// variable globale //////////////////////////////////////////////////////////////////////////////
typedef struct Mesure Mesure;
struct Mesure {
  String coordonnees;
  String date;
  short int temperature;
  short int pression;
  short int hygrometrie;
  short int luminosite;
  Mesure *suivant;
};

typedef struct Liste Liste;
struct Liste {
  Mesure *premier;
};

#define PUSHPIN 2 // bouton rouge
#define PUSHPIN2 3 // bouton vert

#define SEALEVELPRESSURE_HPA (1013.25)
LiquidCrystal lcd = LiquidCrystal ( 8, 9, 10, 11, 12, 13 ) ;
ChainableLED led(6, 7, 5);

uRTCLib rtc(0x68);

short int CurrentMode = 1; //standard
short int PreviousMode = 1; // standard

static unsigned long previousMillis = 0; 
static unsigned long previousMillis_B = 0; 
unsigned long interval = 5000; // 180000000; // 30 min
static unsigned long temps = 0;

void buttoncheck(); // fonction interruption button rouge
void buttoncheck2(); // fonction interruption button vert

void Standard(); // Mode Standard
void Configuration(); // Mode Configuration
void Maintenance(); // Mode Maintenance
void Economique(); // Mode Economique

short int step = 0;

void Blink_test(unsigned long interval, short int a1, short int b1, short int c1, short int a2, short int b2, short int c2);
void write_SD(String file_name, String writing_text);
void read_SD(String file_name);
void display_screen(String texte);
int retour_luminosite();
int BME_value(int val);
String GPS_location();

Liste *initListe();
void ajouterdebut(Liste *liste, String coordonnees, String date, short int temperature, short int pression, short int hygrometrie, short int luminosite);
int compter(Liste *liste);
void lecture(Liste *liste);

Liste *listMesure = initListe();

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////// setup /////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup()
{
  Serial.begin(9600);
  URTCLIB_WIRE.begin();
  rtc.set(00, 57, 8, 4, 20, 10, 22); // (second, minute, hour, dayOfWeek, dayOfMonth, month, year)

  short int LUMIN = 1;
  short int LUMIN_LOW = 255;
  short int LUMIN_HIGH = 768;
  short int TEMP_AIR = 1;
  short int MIN_TEMP_AIR = -10;
  short int MAX_TEMP_AIR = 60;
  short int HYGR = 1;
  short int HYGR_MINT = 0;
  short int HYGR_MAXT = 50;
  short int PRESSURE = 1;
  short int PRESSURE_MIN = 850;
  short int PRESSURE_MAX = 1080;
  EEPROM.put(0, LUMIN);
  EEPROM.put(1, LUMIN_LOW);
  EEPROM.put(2, LUMIN_HIGH);
  EEPROM.put(3, TEMP_AIR);
  EEPROM.put(4, MIN_TEMP_AIR);
  EEPROM.put(5, MAX_TEMP_AIR);
  EEPROM.put(6, HYGR);
  EEPROM.put(7, HYGR_MINT);
  EEPROM.put(8, HYGR_MAXT);
  EEPROM.put(9, PRESSURE);
  EEPROM.put(10, PRESSURE_MIN);
  EEPROM.put(11, PRESSURE_MAX);
  
  pinMode(PUSHPIN, INPUT); // Initialisation bouton rouge
  pinMode(PUSHPIN2, INPUT); // Initialisation bouton vert

  attachInterrupt(digitalPinToInterrupt(PUSHPIN),buttoncheck,CHANGE); 
  attachInterrupt(digitalPinToInterrupt(PUSHPIN2),buttoncheck2,CHANGE);
  supprimer(listMesure);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////// main //////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void loop()
{
  //Serial.println(CurrentMode);
  if (CurrentMode == 1){ // Basculer en mode Standard
    Standard(); 
  }
  else if (CurrentMode == 2){ // Basculer en mode Configuration
    Configuration();
  }
  else if (CurrentMode == 3){ // Basculer en mode Maintenance
    Maintenance(); 
  }
  else if (CurrentMode == 4){ // Basculer en mode Economique
    Economique();
  }
}
////////////////////////////////////////////////////////////////////////////// Fonction d'interruption bouton rouge //////////////////////////////////////////////////////////////////////////////
void buttoncheck()
{
  unsigned long debutAppui = millis();
  if (temps == 0){
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
            if (millis() - temps  > 5000){
              CurrentMode = 3;
              PreviousMode = 1; // Recupérer et stocker le mode précédent
              temps = 0;
            }
            else{
              temps = 0;
            }
          }
          break;
        case 3: // Passer de Maintenance au mode précédent (Standard ou Economique)
          if (millis() - temps  > 5000){
            CurrentMode = PreviousMode;
            PreviousMode = 3; // Recupérer et stocker le mode précédent
            temps = 0;
          }
          else{
            temps = 0;
          }
          break;
        case 4: // Passer de Economique a Standard
          if (millis() - temps  > 5000){
            CurrentMode = 1;
            PreviousMode = 4; // Recupérer et stocker le mode précédent
            temps = 0;
          }
          else{
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
  if (temps == 0){
    temps = millis();
  }
  else {
    if (millis() - temps  < 10) {
      // Anti rebond
    }
    else {
      switch (CurrentMode) {
        case 1:
          if (millis() - temps  > 5000){
            CurrentMode = 4;
            PreviousMode = 1; // Recupérer et stocker le mode précédent
            temps = 0;
          }
          else{
            temps = 0;
          }
          break;
        case 4:
          if (millis() - temps  > 5000){
            CurrentMode = 3;
            PreviousMode = 4; // Recupérer et stocker le mode précédent
            temps = 0;
          }
          else{
            temps = 0;
          }
          break;
      }
    }
  }
}
////////////////////////////////////////////////////////////////////////////// mode standard //////////////////////////////////////////////////////////////////////////////
void Standard()
{
  if ( ( millis() - previousMillis_B ) >= 5000 ) {
    previousMillis_B = millis();
    Serial.println(compter(listMesure));
    if (compter(listMesure) < 10){
      Serial.println("Prise des mesures.. ");
  
      rtc.refresh();
      String date;
      date = String(rtc.day())+ String("/") + String(rtc.month())+ String("/") + String(rtc.year())+ String(" ") + String(rtc.hour())+ String(":") + String(rtc.minute())+ String(":") + String(rtc.second());
      display_screen(date);
    
      display_screen(GPS_location());
    
      display_screen(String("Temperature ") + BME_value(1) + String("c"));
      display_screen(String("Pression ") + BME_value(2) + String("hpa")) ;
      display_screen(String("Hygrometrie ") + BME_value(3) + String("%")) ;
      display_screen(String("Luminosite ") + retour_luminosite() + String("lux"));
  
      ajouterdebut(listMesure, GPS_location(), date, BME_value(1), BME_value(2), BME_value(3), retour_luminosite());
    }
    else{
      Serial.println("Prise geqgsssssssssssss mesures.. ");      
    }
  }
}
////////////////////////////////////////////////////////////////////////////// mode configuration //////////////////////////////////////////////////////////////////////////////

void Configuration()
{
 led.setColorRGB(0, 255, 255, 0); // jaune 
  if(millis() - previousMillis <= interval){ // Boucle 30 min 
    Serial.println("Configuration");
    previousMillis = 0;
  }
  else {  
    CurrentMode = 1; // Basculer a mode Standard apres 30 min
  }
}
////////////////////////////////////////////////////////////////////////////// mode maintenance //////////////////////////////////////////////////////////////////////////////

void Maintenance()
{
  led.setColorRGB(0, 255, 123, 0); // orange
  Serial.println("Maintenance");
}

////////////////////////////////////////////////////////////////////////////// mode economique //////////////////////////////////////////////////////////////////////////////

void Economique()
{
  led.setColorRGB(0, 0, 0, 255); // bleu
  Serial.println("Economique");
}

////////////////////////////////////////////////////////////////////////////// fonction affichage a l'ecran //////////////////////////////////////////////////////////////////////////////

void display_screen(String texte){
  int index = texte.indexOf(' ');
  String texte1 = texte.substring(0,index);
  String texte2 = texte.substring(index+1,-1);
  lcd.begin( 16, 2 );
  lcd.setCursor(round(texte1.length())/2,0);
  lcd.print(texte1);
  lcd.setCursor(round(texte2.length())/2,1);
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
    Blink_2_main(1000, 255, 0, 0, 255, 255, 255);
    //while (1);

    }
  Serial.println("initialization done.");
  myFile = SD.open(file_name, FILE_WRITE);
   if (myFile) {
    Serial.println("Writing to test.txt...");
    Serial.println(writing_text);
    myFile.println(writing_text);
    myFile.close();
    Serial.println("done.");
  } else {
    Serial.println("error opening " + file_name);
  }
  Serial.println("");
 }

////////////////////////////////////////////////////////////////////////////// fonction du capteur de luminosité //////////////////////////////////////////////////////////////////////////////

int retour_luminosite(){
      Serial.println("luminosite");

   int erreur = 1;
   if (erreur == 1){
    Serial.println("erreur");
    Blink_main(1000, 255, 0, 0, 255, 255, 0);
   }
   else{
   Serial.println(analogRead(A2));
    return analogRead(A2);
   }
 }

////////////////////////////////////////////////////////////////////////////// fonction du capteur BME //////////////////////////////////////////////////////////////////////////////

 int BME_value(int val){
  Adafruit_BME280 bme;
  int tab[3];
    if (!bme.begin(0x76)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    Blink_2_main(1000, 255, 0, 0, 0, 255, 0);
    while (1);
  }
  tab[0] = bme.readTemperature();
  tab[1] = bme.readPressure() / 100.0F;
  tab[2] = bme.readHumidity();

  return tab[val - 1];
 }
////////////////////////////////////////////////////////////////////////////// fonction simulant l'appel au GPS //////////////////////////////////////////////////////////////////////////////

 String GPS_location(){
  int erreur = 0;
  if (erreur == 1){
    Serial.println("Erreur acces gps");
    Blink_main(1000, 255, 0, 0, 255, 255, 0);
    return "Erreur city";
    }
  else{
  return "Bordeaux";
  }
 }

////////////////////////////////////////////////////////////////////////////// fonction de clignotement simple //////////////////////////////////////////////////////////////////////////////

 void Blink_main(unsigned long intervall, short int a1, short int b1, short int c1, short int a2, short int b2, short int c2){
  unsigned long currentMillis = millis();
  Serial.print("led");
   Serial.println(String("currentMillis : ") + currentMillis);
  Serial.println(String("previousMillis : ") + previousMillis);
  if(currentMillis - previousMillis >= intervall and step == 0){
    previousMillis = currentMillis;
    Serial.println("step 0");
    led.setColorRGB(0, a1, b1, c1);  
    step = 1;
  }
  if(currentMillis - previousMillis >= intervall and step == 1){
    previousMillis = currentMillis;
        Serial.println("step 1");

    led.setColorRGB(0, a2, b2, c2);  
    step = 0;
  }  
}

////////////////////////////////////////////////////////////////////////////// fonction de clignotement double  //////////////////////////////////////////////////////////////////////////////
 
void Blink_2_main(unsigned long interval, short int a1, short int b1, short int c1, short int a2, short int b2, short int c2){
  unsigned long currentMillis = millis();

  if(currentMillis - previousMillis >= interval and step == 0){
    previousMillis = currentMillis;
    led.setColorRGB(0, a1, b1, c1);  
    step = 1;
  }
  if(currentMillis - previousMillis >= interval and step == 1){
    previousMillis = currentMillis;
    led.setColorRGB(0, a2, b2, c2);  
    step = 2;
  }  

    if(currentMillis - previousMillis >= interval and step == 2){
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

  element->coordonnees = "";
  element->date = "";
  element->temperature = 0;
  element->pression = 0;
  element->hygrometrie = 0;
  element->luminosite = 0;

  element->suivant = NULL;
  liste->premier = element;

  return liste;
}

////////////////////////////////////////////////////////////////////////////// Fonction permettant d'ajouter un element à la liste //////////////////////////////////////////////////////////////////////////////
void ajouterdebut(Liste *liste, String coordonnees, String date, short int temperature, short int pression, short int hygrometrie, short int luminosite) {
  /* Création nouvelle element*/
  Mesure *newData = (Mesure*)malloc(sizeof(*newData));
  
  newData->coordonnees = coordonnees;
  newData->date = date;
  newData->temperature = temperature;
  newData->pression = pression;
  newData->hygrometrie = hygrometrie;
  newData->luminosite = luminosite;

  /* Insertion de l'élément crée au début de la liste */
  newData->suivant = liste->premier;
  liste->premier = newData;
}
////////////////////////////////////////////////////////////////////////////// Fonction permettant de supprimer les elements de la liste //////////////////////////////////////////////////////////////////////////////
void supprimer(Liste *liste)
{
    if (liste == NULL)
    {
        exit(EXIT_FAILURE);
    }

    if (liste->premier != NULL)
    {
        Mesure *aSupprimer = liste->premier;
        liste->premier = liste->premier->suivant;
        free(aSupprimer);
    }
}
////////////////////////////////////////////////////////////////////////////// Fonction permettant de compter le nombre d'element de la liste //////////////////////////////////////////////////////////////////////////////
int compter(Liste *liste){
  int i;
  Mesure *x = liste->premier;
  while (x->suivant != NULL) {
    x = x->suivant;
    i++;
  }
  return i;
}
////////////////////////////////////////////////////////////////////////////// Fonction permettant de lire les données de chaque element de la liste //////////////////////////////////////////////////////////////////////////////
void lecture(Liste *liste) {
  if (liste == NULL)
  {
      exit(EXIT_FAILURE);
  }
  
  Mesure *x = liste->premier;

  while (x != NULL) {
    Serial.println(x->date);
    x = x->suivant;
  }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
