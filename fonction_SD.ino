#include <SPI.h>
#include <SD.h>

void write_SD(String file_name, String writing_text);
void read_SD(String file_name);
 
void setup() {
 Serial.begin(9600);
 write_SD("test.txt", "Et l'informatique c'est génial !!");
 read_SD("test.txt");

}

void loop() {}
/////////////////////////////////////////////////////////////////////
void write_SD(String file_name, String writing_text){
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
