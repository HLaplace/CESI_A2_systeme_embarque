void setup() {
   pinMode(A0, INPUT);
   analogWrite(A0, LOW);

   Serial.begin(9600);
}

void loop() {
   int light = analogRead(A0);
   Serial.print("Light = ");
   Serial.println(light);
  
}
