#define OnOffPB 0
#define UpPB 1
#define DownPB 2
#define ModePB 3

#define ErrorLEd 4

#define Compressor1 5
#define Compressor2 7

#define Fan1 6
#define Fan2 9

bool OnOff = false, Error = false;
long TimeOld, TimeNew, TimeChange;
int Temp = 25, FanSpeed = 10, Time = 60, Mode = 0, AC = 1;

void getTemp() {
  Temp = analogRead(A0);
  Temp = map(Temp, 0, 1023, 0, 100);
  Temp = Temp / 10;
}
void userInterface() {
  if ( digitalRead(OnOffPB) ) {
    OnOff = !OnOff;
  }

  if ( digitalRead(Mode) ) {
    Mode = (Mode++)%3;
  }

  if ( digitalRead(UpPB) ) {
    if(Mode == 0 && Temp < 30) {
      Temp++;
      TimeChange = millis();
    }
    else if(Mode == 1 && FanSpeed < 255) {
      FanSpeed++;
    }
    else if(Mode == 2 && Time < 360) {
      Time+=60;
    }
  }

  if ( digitalRead(DownPB) ) {
    if(Mode == 0 && Temp > 15) {
      Temp--;
      TimeChange = millis();
    }
    else if(Mode == 1 && FanSpeed > 0) {
      FanSpeed--;
    }
    else if(Mode == 2 && Time > 60) {
      Time-=60;
    }
  }
}

void systemBehaviour() {
  if ( OnOff ) {

    if(Error) {
      digitalWrite(ErrorLEd, HIGH);

      if(getTemperature(AC) > Temp) {

        if(AC == 1) {
          digitalWrite(Compressor1, HIGH);
          analogWrite(Fan1, FanSpeed);
        }
        else {
          digitalWrite(Compressor2, HIGH);
          analogWrite(Fan2, FanSpeed);
        }
      }
      else {
        digitalWrite(Compressor1, LOW);
        digitalWrite(Compressor2, LOW);
      }
    }
    else {
      if(millis() - TimeChange > 5min) {
        Error = true;
        
        if(AC == 1){
          AC = 2;
          digitalWrite(Compressor1, LOW);
          analogWrite(Fan1, 0);
        }
        else{
          AC = 1;
          digitalWrite(Compressor2, LOW);
          analogWrite(Fan2, 0);
        }
      }
      
      if(millis() - TimeOld > Time) {
        TimeOld = millis();
        
        if(AC == 1){
          AC = 2;
          digitalWrite(Compressor1, LOW);
          analogWrite(Fan1, 0);
        }
        else{
          AC = 1;
          digitalWrite(Compressor2, LOW);
          analogWrite(Fan2, 0);
        }
      }

      if(getTemperature(AC) > Temp) {

        if(AC == 1) {
          digitalWrite(Compressor1, HIGH);
          analogWrite(Fan1, FanSpeed);
        }
        else {
          digitalWrite(Compressor2, HIGH);
          analogWrite(Fan2, FanSpeed);
        }
      }
      else {
        digitalWrite(Compressor1, LOW);
        digitalWrite(Compressor2, LOW);
        TimeChange = millis();
      }
    }

  }
  else {
    //switch everything off
    digitalWrite(Compressor1, LOW);
    digitalWrite(Compressor2, LOW);
    digitalWrite(Fan1, LOW);
    digitalWrite(Fan2, LOW);
  }
}
void setup() {
  // put your setup code here, to run once:
  pinMode(OnOffPB, INPUT);
  pinMode(UpPB, INPUT);
  pinMode(DownPB, INPUT);
  pinMode(ModePB, INPUT);

  pinMode(ErrorLEd, OUTPUT);

  pinMode(Compressor1, OUTPUT);
  pinMode(Compressor2, OUTPUT);

  pinMode(Fan1, OUTPUT);
  pinMode(Fan2, OUTPUT);

}

void loop() {
  // put your main code here, to run repeatedly:

}
