#define PIN_FUEL  9
#define FUEL_UPDATE_RATE  200

int lastFuelTime = 0;
float fuelLevel = 255.0f;
float fuelRate = 0.0f;

boolean warningSent = false;

int anaVal[] = {
  0,0,0,0};

byte distVal = 0;
byte swVal[12];



void setup(){
  Serial.begin(115200);
  for(int i = 0; i < 12; i++){
    pinMode(22 + i, INPUT);
    digitalWrite(22 + i, HIGH);
    swVal[i] = 0;
  } 

  for(int i = 0; i < 4; i++){
    int b = analogRead(i);    
    anaVal[i] = b;     
  }

  for(int i = 0; i < 12; i++){
    byte b = digitalRead(22 + i);
    delay(10);
    b = b & digitalRead(22 + i);
    swVal[i] = b;

  }

  int t = swVal[7] << 1 | swVal[5];
  t = t-1;
  if(t == 2) {
    t = 1;
  } 
  else if (t == 1){
    t = 2;
  }
  if(distVal != t){
    distVal = t;
  }

  
}
void readSwitches(){

  for(int i = 0; i < 4; i++){
    int b = analogRead(i);
    if(abs(anaVal[i] - b) >= 5){
      anaVal[i] = b;
      Serial.print("P");
      Serial.print(i);
      Serial.print(":");
      Serial.print(map(b,0,1024,999,0));
      Serial.print(",");
    }
  }

  for(int i = 0; i < 12; i++){
    byte b = digitalRead(22 + i);
    delay(10);
    b = b & digitalRead(22 + i);

    if(b != swVal[i]){

      swVal[i] = b;
      if(i != 7 && i != 5){
        Serial.print("PS");
        Serial.print(i);
        Serial.print(":");
        Serial.print(1-b);
        Serial.print(",");
      }
    }
  }

  int t = swVal[7] << 1 | swVal[5];
  t = t-1;
  if(t == 2) {
    t = 1;
  } 
  else if (t == 1){
    t = 2;
  }
  if(distVal != t){
    distVal = t;
    Serial.print("PS13:");
    Serial.print(t );
    Serial.print(",");
  }

}
void loop(){

  readSwitches();
  //update fuel meter
  if(millis() - lastFuelTime > FUEL_UPDATE_RATE){
    
    if(fuelLevel + fuelRate < 50.0f){
      fuelLevel = 50.0f;
      if(!warningSent){
        Serial.print("FE,");
        warningSent = true;
      }
    } else if (fuelLevel + fuelRate > 255.0f){
      fuelLevel = 255.0f;
      if(!warningSent){
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

  delay(10);
  while(Serial.available()){
    char c = Serial.read();
    if(c == 'P'){    //console requested a status update for all switches
      //read out everything
      for(int i = 0; i < 12; i++){
        if(i!=9){    //9 is the grappler arming switch, dont probe it
          swVal[i] = 255;
        }
      }
      anaVal[0] = 2048;
      anaVal[1] = 2048;
      anaVal[2] = 2048;
      anaVal[3] = 2048;
      readSwitches();
      Serial.print("PC,");//probe complete
    } else if ( c == 'F' ){
      char d = Serial.read();
   //   Serial.println ((int)d);
      fuelRate = (((int)d - 128) * 4) / 1000.f;
 //     Serial.println(fuelRate);
    } else if (c == 'f'){    //query the fuel level
      Serial.print((int)fuelLevel);
      Serial.print(",");
    } else if( c == 'R'){
      fuelLevel = 255.0f;
      fuelRate = 0.0f;
    }

  }
}


