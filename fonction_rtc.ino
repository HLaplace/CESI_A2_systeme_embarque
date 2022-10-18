#include "Arduino.h"
#include "uRTCLib.h"

uRTCLib rtc(0x68);

void setup() {
  Serial.begin(9600);
  delay(3000); // wait for console opening

  URTCLIB_WIRE.begin();
  rtc.set(0, 31, 20, 2, 17, 10, 22);
 // rtc.set(second, minute, hour, dayOfWeek, dayOfMonth, month, year)

}

void loop() {
  rtc.refresh();
  Serial.println(String(rtc.day())+ String("/") + String(rtc.month())+ String("/") + String(rtc.year())+ String(" ") + String(rtc.hour())+ String(":") + String(rtc.minute())+ String(":") + String(rtc.second()));
  delay(5000);
  
}
