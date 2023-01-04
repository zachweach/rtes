typedef enum {
  sINIT = 0,
  sSYS_ON = 1,
  sSYS_OFF = 2,
  sVIOLATION = 3,
} state;

typedef struct {
  bool sysOn; // true if the system is on, false otherwise
  bool acOn; // true if the AC is on, false otherwise
  int desTemp; // desired temperature, in centidegrees (range [5400,8600))
  int curTemp;// current/actual temperature, in centidegrees (range [5400,8600))
} sysMsg;

typedef struct {
  bool acOnLed; // true if the hardware signal for the LED is HIGH, false otherwise
  int knobVal;  // the ADC value for the potentiometer (between 0 and 1023, inclusive)
  unsigned long tLastKnobChange; // the last time the potentiometer was turned, in millis
} hwInputs;

typedef struct {
  unsigned long tLastMsg; // the time that the previous message was received
  unsigned long tLastSysChange; // the last time the system toggled between off and on
  bool prevSysVal; // the last observed status of the system (true for on, false for off)
  unsigned long tLastAcChange; // the last time the AC toggled between off and on
  bool prevAcVal; // the last observed status of the AC (true for on, false for off)
  float prevCurTempVal; // the last observed temperature
} monVars;

void stopSystem();
bool checkSysOn(sysMsg msg, hwInputs hw, monVars vars, unsigned long mils);
bool checkSysOff(sysMsg msg, hwInputs hw, monVars vars, unsigned long mils);
bool checkSysToggle(monVars vars, unsigned long mils);
monVars initVars(sysMsg msg, hwInputs hw, unsigned long mils);
monVars updateVars(sysMsg msg, hwInputs hw, monVars vars, unsigned long mils);

void updateMonitor(sysMsg msg, hwInputs hw, unsigned long mils);
