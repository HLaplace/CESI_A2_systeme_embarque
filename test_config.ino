#include <EEPROM.h>

unsigned long interval = 5000; // 180000000; // 30 min
void modification_valeur(int EEPROM_case);
  String tab[12] = {"LUMIN", "LUMIN_LOW", "LUMIN_HIGH", "TEMP_AIR", "MIN_TEMP_AIR", "MAX_TEMP_AIR", "HYGR", "HYGR_MINT", "HYGR_MAXT", "PRESSURE", "PRESSURE_MIN", "PRESSURE_MAX"};

void setup()
{
  Serial.begin(9600);

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
}

void loop()
{
  Serial.println("Passage en mode config ");
  while (Serial.available()){
  String user_input = "";
  while (user_input == ""){
     user_input = Serial.readString();//recupere une chaine de caractere
  }
    user_input.trim();// suppression des espaces
    Serial.print(String("|") + user_input + String("|"));

for ( int i=0; i<11; i++)
   {
  Serial.println(tab[i]);
  if (user_input == tab[i]){
    modification_valeur(i);
    }
}
    if (user_input == "RESET"){
      Serial.print("reset des valeurs");                          
      setup();
      delay(50); 
    }
        else if (user_input == "VERSION"){
      Serial.print("Version 4.0 du programme");
      delay(50);
    }
        else{
      Serial.print("UNKNOW COMMAND");
    }
  }
}
/////////////////////////////////////////////////////////////////////////////////////////
void modification_valeur(int EEPROM_case){
  String new_var = "";
  Serial.print("entrez la nouvelle valeur de la variable : ");
    while (new_var == ""){
     new_var = Serial.readString();//recupere une chaine de caractere
  }
  new_var.trim();
  EEPROM.write(EEPROM_case, new_var.toInt());
  Serial.println("Variable réaffecter avec succes -> " + String(new_var));
  }
