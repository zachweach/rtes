int MON_LED = 7;
int AC_LED = 4;
int POT = A1;

void setup() {
  Serial.begin(9600);
  while (!Serial);
  Serial1.begin(9600);
  Serial1.setTimeout(10);
  pinMode(7, OUTPUT);
  pinMode(4, INPUT);
}

void loop() {
  static int i = 0;
  static char curBuf[8];

  if (i < 10) {
    digitalWrite(MON_LED, HIGH); 
    i++;
  } else if (i < 20) {
    digitalWrite(MON_LED, LOW);
    i++;
    if (i == 20) {
      i = 0;
    }
  }
  
  int bytesRead = Serial1.readBytes(curBuf, 8);
  
  if (bytesRead == 8) {
    Serial.print("Serial received (\"");
    Serial.print(curBuf);
    Serial.println("\")");

    Serial.print("Potentiometer: ");
    Serial.println(analogRead(POT));
    if (digitalRead(AC_LED) == HIGH) {
      Serial.println("AC LED on");
    } else {
      Serial.println("AC LED off");
    }
    Serial.println("");
  }
  delay(40);
}
