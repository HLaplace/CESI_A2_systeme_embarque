#include <SPI.h>
#include <SD.h>

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
