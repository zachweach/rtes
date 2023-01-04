#include "lab8_monitor.h"

void setup() {
  Serial.begin(9600);
  while (!Serial);
  Serial.println("Starting system");
  Serial1.begin(9600);
  Serial1.setTimeout(10);
  pinMode(7, OUTPUT); // the status LED
  pinMode(4, INPUT);  // the AC LED
  digitalWrite(7, HIGH); // the potentiometer
}

void loop() {
  static int prevPotVal = analogRead(A1);
  static unsigned long lastPotChange = millis();
  static char curBuf[14];
  
  delay(40);
  bool doMonitor = false; // we only want to monitor if we've received a message
  sysMsg receivedMsg;

  int bytesRead = Serial1.readBytes(curBuf, 14);

  /*
   * Message received:
   * process character-by-charater, with error checking (skip messages that do not fit the format)
   * Format is
   * a b cccc dddd
   * a: {0, 1} (sys on)
   * b: {0, 1} (ac on)
   * c: 4-digit number (desired temperature)
   * d: 4-digit number (current temperature)
   */
  if (bytesRead == 14) {
    if (curBuf[13] != '\n') {
      return;
    }
    curBuf[13] = 0;
    if (!(curBuf[1] == ' ' && curBuf[3] == ' ' && curBuf[8] == ' ')) {
      return; // skip malformed message
    }
      
    if (!(curBuf[0] == '0' or curBuf[0] == '1') or !(curBuf[2] == '0' or curBuf[2] == '1')) {
      return; // skip malformed message
    }
    receivedMsg.sysOn = curBuf[0] == '1';
    receivedMsg.acOn = curBuf[2] == '1';
      
    if (!(isDigit(curBuf[4]) && isDigit(curBuf[5]) && isDigit(curBuf[6]) && isDigit(curBuf[7]))) {
      return; // skip malformed message
    }
    unsigned int desTemp = (unsigned int) (curBuf[4] - '0') * 1000;
    desTemp += (unsigned int) (curBuf[5] - '0') * 100;
    desTemp += (unsigned int) (curBuf[6] - '0') * 10;
    desTemp += (unsigned int) (curBuf[7] - '0');
    receivedMsg.desTemp = desTemp;

    if (!(isDigit(curBuf[9]) && isDigit(curBuf[10]) && isDigit(curBuf[11]) && isDigit(curBuf[12]))) {
      return; // skip malformed message
    }
    unsigned int curTemp = (unsigned int) (curBuf[9] - '0') * 1000;
    curTemp += (unsigned int) (curBuf[10] - '0') * 100;
    curTemp += (unsigned int) (curBuf[11] - '0') * 10;
    curTemp += (unsigned int) (curBuf[12] - '0');
    receivedMsg.curTemp = curTemp;
      
    Serial.print("\nTime: ");
    Serial.println(millis());
    Serial.print("Status message: ");
    Serial.println(curBuf);
    doMonitor = true;
  }

  // check to see if the potentiometer has moved, and update accordingly
  int potVal = (int) analogRead(A1);
  if (abs(potVal - prevPotVal) > 10) { // account for some noise
    prevPotVal = potVal;
    lastPotChange = millis();
  }

  // transition the FSM
  if(doMonitor) {
    Serial.print("AC LED: ");
    bool ledVal = digitalRead(4) == HIGH;
    Serial.println(ledVal);
    
    Serial.print("Potentiometer: ");
    Serial.println(potVal);

    updateMonitor(receivedMsg, {ledVal, potVal, lastPotChange}, millis());
  }
}
