#define PIN_FUEL  9
#define FUEL_UPDATE_RATE  200

#define STICK_BASE  46
#define DNP_LED     51
#define DNP_BTN     53

#define NUMSWITCH  17
int lastFuelTime = 0;
float fuelLevel = 255.0f;
float fuelRate = 0.0f;

boolean warningSent = false;

int anaVal[] = {
  0, 0, 0, 0
};

byte distVal = 0;
byte swVal[NUMSWITCH];

int pinMap[] = { 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, DNP_BTN, 46, 48, 50, 52 };

boolean dnpState = false;
boolean dnpBlinking = false;
int dnpTimeout = 2;

/*
ps13 -> ps14 in hardwarecontroller

12 is dnp
11 is jam
*/
void setup() {
  Serial.begin(115200);
  for (int i = 0; i < NUMSWITCH; i++) {
    pinMode(pinMap[i], INPUT);
    digitalWrite(pinMap[i], HIGH);

    swVal[i] = 0;
  }

  for (int i = 0; i < 4; i++) {
    int b = analogRead(i);
    anaVal[i] = b;
  }

  for (int i = 0; i < NUMSWITCH; i++) {
    byte b = digitalRead(pinMap[i]);
    delay(10);
    b = b & digitalRead(pinMap[i]);
    swVal[i] = b;
  }

  int t = swVal[7] << 1 | swVal[5];
  t = t - 1;
  if (t == 2) {
    t = 1;
  } else if (t == 1) {
    t = 2;
  }
  if (distVal != t) {
    distVal = t;
  }
  //DNP button
  pinMode(DNP_LED, OUTPUT);
  pinMode(DNP_BTN, INPUT);
  digitalWrite(DNP_BTN, HIGH);
}
void readSwitches() {
  for (int i = 0; i < 4; i++) {
    int b = analogRead(i);
    if (abs(anaVal[i] - b) >= 5) {
      anaVal[i] = b;
      Serial.print("P");
      Serial.print(i);
      Serial.print(":");
      Serial.print(map(b, 0, 1024, 999, 0));
      Serial.print(",");
    }
  }

  for (int i = 0; i < NUMSWITCH; i++) {
    byte b = digitalRead(pinMap[i]);
    delay(10);
    b = b & digitalRead(pinMap[i]);

    if (b != swVal[i]) {
      swVal[i] = b;
      if (i != 7 && i != 5) {
        Serial.print("PS");
        Serial.print(i);
        Serial.print(":");
        Serial.print(1 - b);
        Serial.print(",");
      }
    }
  }

  int t = swVal[7] << 1 | swVal[5];
  t = t - 1;
  if (t == 2) {
    t = 1;
  } else if (t == 1) {
    t = 2;
  }
  if (distVal != t) {
    distVal = t;
    Serial.print("PS14:");
    Serial.print(t);
    Serial.print(",");
  }
}
void loop() {
  readSwitches();
  //update fuel meter
  if (millis() - lastFuelTime > FUEL_UPDATE_RATE) {
    if (fuelLevel + fuelRate < 50.0f) {
      fuelLevel = 50.0f;
      if (!warningSent) {
        Serial.print("FE,");
        warningSent = true;
      }
    } else if (fuelLevel + fuelRate > 255.0f) {
      fuelLevel = 255.0f;
      if (!warningSent) {
        Serial.print("FF,");
        warningSent = true;
      }
    } else {
      warningSent = false;
      fuelLevel += fuelRate;
      // Serial.println(fuelLevel);
    }

    lastFuelTime = millis();
    analogWrite(PIN_FUEL, (int)fuelLevel);
  }
  if (dnpBlinking) {
    if (dnpTimeout <= 0) {
      dnpState = !dnpState;
      dnpTimeout = 2;
    }
    dnpTimeout--;
    digitalWrite(DNP_LED, dnpState == true ? HIGH : LOW);
  } else {
    digitalWrite(DNP_LED, LOW);
  }

  delay(10);
  while (Serial.available()) {
    char c = Serial.read();
    if (c == 'P') {  //console requested a status update for all switches
      //read out everything
      for (int i = 0; i < NUMSWITCH; i++) {
        if (i != 9) { //9 is the grappler arming switch, dont probe it
          swVal[i] = 255;
        }
      }
      anaVal[0] = 2048;
      anaVal[1] = 2048;
      anaVal[2] = 2048;
      anaVal[3] = 2048;
      readSwitches();
      Serial.print("PC,");//probe complete
    } else if (c == 'F') {
      int d = Serial.read();
      //Serial.println (d, DEC);
      fuelRate = ((d - 128) * 4) / 1000.f;
      //Serial.println(fuelRate);
    } else if (c == 'f') {   //query the fuel level
      Serial.print((int)fuelLevel);
      Serial.print(",");
    } else if (c == 'X') {
      fuelRate = 0;
    } else if (c == 'R') {
      fuelLevel = 255.0f;
      fuelRate = 0.0f;
    } else if (c == 'D') {
      char state = Serial.read();
      if (state == '1') {
        dnpBlinking = true;
        dnpState = true;
        dnpTimeout = 2;
      } else {
        dnpBlinking = false;
      }
    }
  }
}
