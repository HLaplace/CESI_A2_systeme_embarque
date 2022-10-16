#include <LiquidCrystal.h>
LiquidCrystal lcd = LiquidCrystal ( 2, 3, 4, 5, 6, 7 ) ;

void setup()
{

}

void loop()
{
  lcd.begin( 16, 2 ) ;
  lcd.setCursor(0,0);
  lcd.print("test");
  delay(5000);
  lcd.clear();
  
}
