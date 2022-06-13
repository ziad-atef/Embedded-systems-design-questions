#define ON_OFF_PB 0
#define UP_PB 1
#define DOWN_PB 2
#define MODE_PB 3

#define ALARM_LED 4

#define COMPRESSOR_PIN 5

#define FAN_SPEED_PIN A0
#define MAX_NUMBER_FAN_LEVELS 5

#define temperature_sensor_1 A0
#define temperature_sensor_2 A1

# define MAX_NUMBER_AC 8
#define addressAC0 6
#define addressAC0 7
#define addressAC0 8

#define MODE_TEMPERATURE 0
#define MODE_FAN_SPEED 1
#define MODE_currentAC 2

bool alarmState = false;
long temperatureTimeOfChange;
int  requiredTemp = 25, currentFanLevel = 0, currentFanSpeed = 10, alternationTime = 60, Mode = 0, currentAC = AC0;

float readTemperatureSensor(int sensor) {
  float voltage = analogRead(sensor) * (5.0 / 1023.0);
  return (voltage-1.375) / 0.0225;
}
float calculateTemperature() {
  return (readTemperatureSensor(temperature_sensor_1) + readTemperatureSensor(temperature_sensor_2)) / 2;
}
void controlAC(int fanSpeed, int compressor) {
  analogWrite(FAN_SPEED_PIN, fanSpeed);
  analogWrite(COMPRESSOR_PIN, compressor);
}
void updateFanLevel(int inc) {
  currentFanLevel += inc % MAX_NUMBER_FAN_LEVELS;
  currentFanSpeed = currentFanSpeed < 0 ? 0 : currentFanSpeed;
  currentFanSpeed = currentFanLevel * (255.0 / MAX_NUMBER_FAN_LEVELS)
}
void updateCurrentAC(int incrementValue)
{
  temperatureTimeOfChange = millis(); // calculate new value for the newly switched on AC
  // update the value of the currentAC
  currentAC += inc % MAX_NUMBER_AC;
  currentAC = currentAC < 0 ? 0 : currentAC;
  // reflect the value on the selection pins
  digitalWrite(addressAC0, currentAC & 0x1); // bit 0 -> xx(x) & 001
  digitalWrite(addressAC1, currentAC & 0x2); // bit 1 -> x(x)x & 010
  digitalWrite(addressAC2, currentAC & 0x4); // bit 2 -> (x)xx & 100
  // Now, I can say that I am proud of myself <3
}
int readPushButton(int pb)
{
  if (digitalWrite(pb) == HIGH)
  {
    while(digitalWrite(pb) == HIGH);
    return 1;
  }
  return 0;
}
void userInterface() {
  if ( readPushButton(Mode) ) {
    Mode = (Mode++)%3;
  }
  int inc  = 0;
  inc = readPushButton(UP_PB) ? 1 : inc;
  inc = readPushButton(DOWN_PB) ? -1 : inc;

  if (inc != 0) {
      if(Mode == MODE_TEMPERATURE) {
        requiredTemp += inc;
        temperatureTimeOfChange = millis(); // reset the time when the temperature is changed
      }
      else if(Mode == MODE_FAN_SPEED) {
        updateFanLevel(inc);
      }
      else if(Mode == MODE_currentAC) {
        updateCurrentAC(inc);
      }
    }
}

void systemBehavior() {
  
  digitalWrite(ALARM_LED, alarmState);
  long long currentTime = millis(); // get the currentTime
  // check if we need to alternate ACs
  if (currentTime - temperatureTimeOfChange > 60min) // 60 minutes has passed
  {
    // alternate if we haven't met the requiredTemp 
    if (calculateTemperature() > requiredTemp) 
    {
      updateCurrentAC(inc);
      alarmState = HIGH; // alarm is on 
      temperatureTimeOfChange = currentTime; // update the temperatureTimeOfChange
    }
  }
  ////////////////////////////////////////////////////////////////////////////////////////////////
  // check if we need to close the ACs
  if (calculateTemperature() <= requiredTemp)
  {
    temperatureTimeOfChange = currentTime; // update the temperatureTimeOfChange
    controlAC(fanSpeed, LOW);
  }
  else 
  {
    controlAC(fanSpeed, HIGH);
    alarmState = LOW; // switch off the alarm
  }
}
void setup() {
  pinMode(UP_PB, INPUT);
  pinMode(DOWN_PB, INPUT);
  pinMode(MODE_PB, INPUT);

  pinMode(ALARM_LED, OUTPUT);

  // ACs (3 bits for 8 ACs)
  pinMode(addressAC0, OUTPUT);
  pinMode(addressAC1, OUTPUT);
  pinMode(addressAC2, OUTPUT);
}

void loop() {
  userInterface();
  systemBehaviour();
}