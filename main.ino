#include <SPI.h>
#include <SD.h>
#include <LiquidCrystal.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define SEALEVELPRESSURE_HPA (1013.25)
LiquidCrystal lcd = LiquidCrystal ( 2, 3, 4, 5, 6, 7 ) ;

void write_SD(String file_name, String writing_text);
void read_SD(String file_name);
void display_screen(String texte);
int retour_luminosite();
int BME_value(int val);

/////////////////////////////////////////////////////////////////////

void setup() {
 Serial.begin(9600);
 int tab[4];
 
 tab[0] = BME_value(1); // temperature
 display_screen(tab[0] ) ;
 delay(1000);

 tab[1] = BME_value(2); // pression
 display_screen(tab[1]) ;
 delay(1000);

 tab[2] = BME_value(3); // humidite
 display_screen(tab[2]) ;
 delay(1000);

 tab[3] = retour_luminosite(); // luminosité
 display_screen(tab[3]) ;
 delay(1000);

 write_SD("test.txt", tab);
 read_SD("test.txt");
}

/////////////////////////////////////////////////////////////////////
void loop() {}
/////////////////////////////////////////////////////////////////////
void write_SD(String file_name, int writing_text){
  File myFile;
  Serial.print("Initializing SD card...");
  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");
  myFile = SD.open(file_name, FILE_WRITE);
   if (myFile) {
    Serial.print("Writing to test.txt...");
    myFile.print(writing_text);
    myFile.close();
    Serial.println("done.");
  } else {
    Serial.println("error opening " + file_name);
  }
 }
/////////////////////////////////////////////////////////////////////
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
}
/////////////////////////////////////////////////////////////////////
void display_screen(int texte){
  lcd.begin( 16, 2 ) ;
  lcd.setCursor(0,0);
  lcd.print(texte);
  //delay(5000);
  lcd.clear();
}
/////////////////////////////////////////////////////////////////////
int retour_luminosite(){
  int var_retour_lumi = analogRead(A0);
   //Serial.print("Light = ");
   //Serial.println(light);
   return var_retour_lumi;
  }
/////////////////////////////////////////////////////////////////////
int BME_value(int val){
  Adafruit_BME280 bme;
  int tab[3];
    if (!bme.begin(0x76)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }
  tab[0] = bme.readTemperature();
  tab[1] = bme.readPressure() / 100.0F;
  tab[2] = bme.readHumidity();

  return tab[val - 1];
  }
/////////////////////////////////////////////////////////////////////
