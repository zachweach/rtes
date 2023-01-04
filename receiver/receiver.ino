void setup() {
  Serial.begin(9600);  // Initialize serial monitor
  while (!Serial);     // Wait for serial monitor to start (need this for USB serial)
  Serial1.begin(9600); // Initialize TX/RX communication (do not need to wait)
}

void loop() {
  if (Serial1.available() > 0) {
    // read the incoming byte:
    byte b = Serial1.read();

    // say what you got:
    Serial.print("I received: ");
    Serial.println((char) b);
  }
}
