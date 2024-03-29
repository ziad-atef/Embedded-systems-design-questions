// Push Buttons
#define ON_OFF_PB 0
#define UP_PB 1
#define DOWN_PB 2
#define MODE_PB 3
// Modes
#define MODE_TEMPERATURE 0
#define MODE_FAN_SPEED 1
#define MODE_ALTERNATING_INTERVAL 2
// States
#define OFF_STATE 0
#define ON_STATE 1

// Alarm LED
#define ALARM_LED 4

// Related to the ACs
#define AC0 0
#define AC1 1
#define Compressor1 5
#define Compressor2 7
#define Fan1 10 // PWM
#define Fan2 11 // PWM
// Temperature sensors
#define temperature_sensor_AC1_1 A0
#define temperature_sensor_AC1_2 A1
#define temperature_sensor_AC0_1 A2
#define temperature_sensor_AC0_2 A3

// Related to threshold after which compressor is activated again
#define COMPRESSOR_THRESHOLD_TEMPERATURE 3
// Global Variables =========================================================================================
bool OnOff = false, alarmState = false;
long timeOfSwitch, temperatureTimeOfChange;
int  requiredTemp = 25, currentFanSpeed = 10, alternationTime = 60, Mode = 0, currentAC = AC1, Capture = 0;
// Utilities =========================================================================================
float readTemperatureSensor(int sensor) {
    return analogRead(sensor) * (5.0 / 1023.0) * 100;
}
float calculateTemperature(int AC) {
    if (AC == AC1) {
        return (readTemperatureSensor(temperature_sensor_AC1_1) + readTemperatureSensor(temperature_sensor_AC1_2)) / 2;
    } else if (AC == AC0) {
        return (readTemperatureSensor(temperature_sensor_AC0_1) + readTemperatureSensor(temperature_sensor_AC0_2)) / 2;
    }
}
void controlAC(int AC, int fanSpeed, int compressor) {
    if (AC == AC1) {
        analogWrite(Fan1, fanSpeed);
        digitalWrite(Compressor1, compressor);
    } else if (AC == AC0) {
        analogWrite(Fan2, fanSpeed);
        digitalWrite(Compressor2, compressor);
    }
}
int readPushButton(int pb){
  if (digitalWrite(pb) == HIGH)
  {
    while(digitalWrite(pb) == HIGH);
    return 1;
  }
  return 0;
}
// Solution =========================================================================================
void userInterface() {
  if ( readPushButton(ON_OFF_PB) ) {
    OnOff = !OnOff;
  }

  if ( readPushButton(Mode) ) {
    Mode = (Mode++)%3;
  }
  int inc  = 0;
  inc = readPushButton(UP_PB) ? 1 : inc;
  inc = readPushButton(DOWN_PB) ? -1 : inc;

  if (inc != 0) {
      if(Mode == MODE_TEMPERATURE) {
        requiredTemp += inc;
        temperatureTimeOfChange = millis()/(60e3); // reset the time when the temperature is changed
        Capture = 0; // reset the capture to allow starting the compressor
      }
      else if(Mode == MODE_FAN_SPEED) {
        currentFanSpeed += inc;
        currentFanSpeed = currentFanSpeed > 255 ? 255 : currentFanSpeed;
        currentFanSpeed = currentFanSpeed < 0 ? 0 : currentFanSpeed;
      }
      else if(Mode == MODE_ALTERNATING_INTERVAL) {
        alternationTime += inc * 60; // multiples of 60 minutes
        alternationTime = alternationTime < 0 ? 0 : alternationTime;
      }
    }

}
void systemBehavior() {
  if ( OnOff == OFF_STATE ) {
    //switch everything off
    controlAC(AC1, LOW, LOW);
    controlAC(AC0, LOW, LOW);
    digitalWrite(ALARM_LED, LOW);
  }
  else {
      digitalWrite(ALARM_LED, alarmState);
      long long currentTime = millis()/(60e3); // get the currentTime in minutes
     // check if we need to alternate ACs
     if (currentTime - timeOfSwitch > alternationTime)
      {
        controlAC(currentAC, LOW, LOW); // close the currentAC
        currentAC = != currentAC; // alternate ACs
        temperatureTimeOfChange = currentTime; // update the temperatureTimeOfChange
        timeOfSwitch = currentTime; // update the timeOfSwitch
      }
      else if (currentTime - temperatureTimeOfChange > 5min) // 5 minutes has passed
      {
        // alternate if we haven't met the requiredTemp and we are not in the middle of an alternation
        if (calculateTemperature(currentAC) > requiredTemp) 
        {
          controlAC(currentAC, LOW, LOW); // close the currentAC
          currentAC = != currentAC; // alternate ACs
          alarmState = HIGH; // alarm is on 
          temperatureTimeOfChange = currentTime; // update the temperatureTimeOfChange
          timeOfSwitch = currentTime; // update the timeOfSwitch
        }
      }
      ////////////////////////////////////////////////////////////////////////////////////////////////
      // check if we need to close the ACs
      int temperature = calculateTemperature(currentAC);
      if (temperature >= requiredTemp && temperature - Capture > COMPRESSOR_THRESHOLD_TEMPERATURE)
      {
        controlAC(currentAC, fanSpeed, HIGH);
        Capture = 0; // reset the capture variable for the compressor to continue operating until we meet the requiredTemp
        if(temperature == requiredTemp)
        {
          Capture = temperature;
        }
      }
      else 
      {
        temperatureTimeOfChange = currentTime; // update the temperatureTimeOfChange
        controlAC(currentAC, fanSpeed, LOW);
        alarmState = LOW; // switch off the alarm
      }
  }
}
// Arduino =========================================================================================
void setup() {
  pinMode(ON_OFF_PB, INPUT);
  pinMode(UP_PB, INPUT);
  pinMode(DOWN_PB, INPUT);
  pinMode(MODE_PB, INPUT);

  pinMode(ALARM_LED, OUTPUT);

  pinMode(Compressor1, OUTPUT);
  pinMode(Compressor2, OUTPUT);
}

void loop() {
  userInterface();
  systemBehaviour();
}
