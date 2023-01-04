void updateMonitor(sysMsg msg, hwInputs hw, unsigned long mils) {
  static state monState = sINIT; // static variables are initialized once, at declaration
  static monVars vars = initVars(msg, hw, mils);
  static const unsigned long monitorStart = mils;

  switch(monState) {
    case sINIT:
    if (mils - monitorStart < 2000 && msg.sysOn != vars.prevSysVal) { // 1-1 a
      vars = initVars(msg, hw, mils);
      monState = sINIT;
    } else if (mils - monitorStart < 2000 && msg.sysOn == vars.prevSysVal) { // 1-1 b
      vars = updateVars(msg, hw, vars, mils);
      monState = sINIT;
    } else if (msg.sysOn && !vars.prevSysVal) { // no timeout; switching from off to on (1-2 a)
      Serial.println("\n*****\nMONITORING SYSTEM ON\n*****");
      vars = initVars(msg, hw, mils);
      monState = sSYS_ON;
    } else if (msg.sysOn && vars.prevSysVal) { // no timeout; on (1-2 b)
      Serial.println("\n*****\nMONITORING SYSTEM ON\n*****");
      vars = updateVars(msg, hw, vars, mils);
      monState = sSYS_ON;
    } else if (!msg.sysOn && !vars.prevSysVal) { // no timeout; off (1-3 b)
      Serial.println("\n*****\nMONITORING SYSTEM OFF\n*****");
      vars = updateVars(msg, hw, vars, mils);
      monState = sSYS_OFF;
    } else { // no timeout; switching from on to off (1-3 a)
      Serial.println("\n*****\nMONITORING SYSTEM OFF\n*****");
      vars = initVars(msg, hw, mils);
      monState = sSYS_OFF;
    }
    break;

    
    case sSYS_ON:
    Serial.println(checkSysOn(msg, hw, vars, mils));
    if (msg.sysOn && !checkSysOn(msg, hw, vars, mils)) { // 2-2
      vars = updateVars(msg, hw, vars, mils);
      monState = sSYS_ON;
    } else if (!msg.sysOn && !checkSysToggle(vars, mils)) { // 2-3
      Serial.println("\n*****\nMONITORING SYSTEM OFF\n*****");
      vars = initVars(msg, hw, mils);
      monState = sSYS_OFF;
    } else { // 2-4
      stopSystem();
      monState = sVIOLATION;
    }
    break;

    case sSYS_OFF:
    if (msg.sysOn && !checkSysToggle(vars, mils)) { // 3-2
      Serial.println("\n*****\nMONITORING SYSTEM ON\n*****");
      vars = initVars(msg, hw, mils);
      monState = sSYS_ON;
    } else if (!msg.sysOn && !checkSysOff(msg, hw, vars, mils)) { // 3-3
      vars = updateVars(msg, hw, vars, mils);
      monState = sSYS_OFF;
    } else { // 3-4
      stopSystem();
      monState = sVIOLATION;
    }
    break;

    case sVIOLATION:
    monState = sVIOLATION;
    break;
  }
}

/*
 * Returns true if some property was violated when system was toggling between on and off
 */
bool checkSysToggle(monVars vars, unsigned long mils) {
  // TODO: change this check so that the property gets violated during normal operation,
  // and then revert the change
  if ((mils - vars.tLastSysChange) < 750) {
    Serial.println("Violation of property 2");
    return true;
  }
  return false;
}

/*
 * Returns true if some property was violated when the system was off
 */
bool checkSysOff(sysMsg msg, hwInputs hw, monVars vars, unsigned long mils) {
  if (mils - vars.tLastMsg > 290) {
    Serial.println("Violation of property 1");
    return true;
  }
  if (msg.sysOn || msg.acOn || msg.desTemp != 0 || msg.curTemp != 0 || hw.acOnLed) {
    Serial.println("Violation of property 3");
    return true;
  }
  return false;
}

bool checkCalc(int knob, int des) {
  int calcVal = 5400 + 25 * knob / 8;
  return (des <= (calcVal + 300)) && (des >= (calcVal - 300));
}

/*
 * Returns true if some property was violated when the system was on
 */
bool checkSysOn(sysMsg msg, hwInputs hw, monVars vars, unsigned long mils) {
  if (msg.desTemp < 5400 || msg.desTemp > 8600 || msg.curTemp < 5400 || msg.curTemp > 8600) {
    Serial.println("Violation of property 4");
    return true;
  }
  if (mils - vars.tLastMsg > 290) {
    Serial.println("Violation of property 1");
    return true;
  }
  if ((vars.prevAcVal != msg.acOn) && (mils - vars.tLastAcChange < 1550)) { // if broken, switch to >=
    Serial.println("Violation of property 5");
    return true;
  }
  if (abs(msg.curTemp - vars.prevCurTempVal) > 60) {
    Serial.println("Violation of property 6");
    return true;
  }
  if (hw.acOnLed != msg.acOn) {
    Serial.println("Violation of property 7");
    return true;
  }
  if ((mils - hw.tLastKnobChange >= 290) && !checkCalc(hw.knobVal, msg.desTemp)) {
    Serial.println("Violation of property 8");
    return true;
  }
  return false;
}

/*
 * Reset variables (when system or AC toggles between on/off)
 */
monVars initVars(sysMsg msg, hwInputs hw, unsigned long mils) {
  monVars returnVars;
  returnVars.tLastMsg = mils;
  returnVars.tLastSysChange = mils;
  returnVars.prevSysVal = msg.sysOn;
  returnVars.tLastAcChange = mils;
  returnVars.prevAcVal = msg.acOn,
  returnVars.prevCurTempVal = msg.curTemp;
  return returnVars;
}

/*
 * Update variables (when system doesn't change state)
 */
monVars updateVars(sysMsg msg, hwInputs hw, monVars vars, unsigned long mils) {
  monVars returnVars;
  // TODO: populate returnVars based on previous vars values and function inputs
  returnVars.tLastMsg = mils;

  returnVars.tLastSysChange = vars.tLastSysChange;
  returnVars.prevSysVal = vars.prevSysVal;
  
  if (msg.acOn != vars.prevAcVal) {
    returnVars.tLastAcChange = mils;
  } else {
    returnVars.tLastAcChange = vars.tLastAcChange;
  }
  returnVars.prevAcVal = msg.acOn,
  
  returnVars.prevCurTempVal = msg.curTemp;
  return returnVars;
}

/*
 * Stop the system (for now, turns off indicator LED and stops the monitor)
 */
void stopSystem() {
  digitalWrite(7, LOW);
  Serial.println("----------");
  while(true) {
    delay(1000);
  }
}
