#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define SEALEVELPRESSURE_HPA (1013.25)

int BME_value(int val);

void setup() {
  Serial.begin(9600);
}

void loop() {
  Serial.println(BME_value(2));
  delay(1000);
}

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
