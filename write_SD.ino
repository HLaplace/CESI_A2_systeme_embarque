#include <SPI.h>
#include <SD.h>

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
