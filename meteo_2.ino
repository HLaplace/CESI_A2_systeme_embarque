#include <SPI.h>
#include <SD.h>
#include <LiquidCrystal.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "Arduino.h"
#include "uRTCLib.h"
#include <ChainableLED.h>

#define SEALEVELPRESSURE_HPA (1013.25)
LiquidCrystal lcd = LiquidCrystal ( 2, 3, 4, 5, 6, 7 ) ;

ChainableLED led(7, 8, 5);
short int step = 0;   // a variable to keep track of what step you're on.

unsigned long previousMillis = 0;

uRTCLib rtc(0x68);
void Blink_test(unsigned long interval, short int a1, short int b1, short int c1, short int a2, short int b2, short int c2);
void write_SD(String file_name, String writing_text);
void read_SD(String file_name);
void display_screen(String texte);
int retour_luminosite();
int BME_value(int val);
String GPS_location();

/////////////////////////////////////////////////////////////////////

void setup() {
 Serial.begin(9600);
 Serial.println("Phase initialisation.. ");
 URTCLIB_WIRE.begin();

 rtc.set(0, 15, 7, 3, 18, 10, 22);
 // rtc.set(second, minute, hour, dayOfWeek, dayOfMonth, month, year)
 delay(1000);
 Serial.println("OK");

 Serial.println("Prise des mesures.. ");
 // sequence de prise de mesure

 // localisation
 display_screen(GPS_location());
 write_SD("test.txt", String(GPS_location()));
 //delay(1000);
 
 // date et heure
 rtc.refresh();
 String date;
 date = String(rtc.day())+ String("/") + String(rtc.month())+ String("/") + String(rtc.year())+ String(" ") + String(rtc.hour())+ String(":") + String(rtc.minute())+ String(":") + String(rtc.second());
 display_screen(date);
 write_SD("test.txt", date);
 //delay(1000);
  
 display_screen(String("temperature ") + BME_value(1) + String("c"));
 write_SD("test.txt", String(BME_value(1)));
 //delay(1000);

 display_screen(String("pression ") + BME_value(2) + String(" hpa")) ;
 write_SD("test.txt", String(BME_value(2)));
 //delay(1000);

 display_screen(String("hygrometrie ") + BME_value(3) + String("%")) ;
 write_SD("test.txt", String(BME_value(3)));
 //delay(1000);

 display_screen(String("lumino ") + retour_luminosite() + String(" lux")) ;
 write_SD("test.txt", String(retour_luminosite()));
 //delay(1000);
 Serial.print("OK");

 Serial.print("Retour carte SD");
 //read_SD("test.txt");
 Serial.print("CLoture");

}

/////////////////////////////////////////////////////////////////////
void loop() {}
/////////////////////////////////////////////////////////////////////
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
/////////////////////////////////////////////////////////////////////
/*
void read_SD(String file_name){
  File myFile;
  myFile = SD.open(file_name);
  if (myFile) {
    Serial.println(file_name);
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    myFile.close();
  } else {
    Serial.println("error opening test.txt");
  }
}*/
/////////////////////////////////////////////////////////////////////
void display_screen(String texte){
  lcd.begin( 16, 2 ) ;
  lcd.setCursor(0,0);
  lcd.print(texte);
  delay(1000);
  lcd.clear();
}
/////////////////////////////////////////////////////////////////////
int retour_luminosite(){
   int erreur = 1;
   if (erreur == 1){
    Blink_main(1000, 255, 0, 0, 255, 255, 0);
   }
   else{
    return analogRead(A0);
   }
  }
/////////////////////////////////////////////////////////////////////
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
/////////////////////////////////////////////////////////////////////
String GPS_location(){
  int erreur = 0;
  if (erreur == 1){
    Serial.println("Erreur acces gps");
    Blink_main(1000, 255, 0, 0, 255, 255, 0);
    return "Erreur city";
    }
  else{
  return "La Rochelle";
  }
  }
/////////////////////////////////////////////////////////////////////
void Blink_main(unsigned long interval, short int a1, short int b1, short int c1, short int a2, short int b2, short int c2){
  unsigned long currentMillis = millis();

  if(currentMillis - previousMillis >= interval and step == 0){
    previousMillis = currentMillis;
    led.setColorRGB(0, a1, b1, c1);  
    step = 1;
  }
  if(currentMillis - previousMillis >= interval and step == 1){
    previousMillis = currentMillis;
    led.setColorRGB(0, a2, b2, c2);  
    step = 0;
  }  
}
/////////////////////////////////////////////////////////////////////
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
/////////////////////////////////////////////////////////////////////
