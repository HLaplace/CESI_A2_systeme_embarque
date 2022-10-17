
const int buttonPin_1 = 2;  
const int buttonPin_2 = 3;    

void setup() {

  pinMode(buttonPin_1, INPUT);
  pinMode(buttonPin_2, INPUT);
  Serial.begin(9600);
}

void loop() {

  if (digitalRead(buttonPin_1) == LOW) {
    Serial.println("buttonPin_1");
    }
  else if (digitalRead(buttonPin_2) == LOW){
    Serial.println("buttonPin_2");
      }  
  else{Serial.println("NO button");}
}
