
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
  } else if (t == 1){
    t = 2;
  }
  if(distVal != t){
    distVal = t;
  }

  
  
  
}

void loop(){

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
  } else if (t == 1){
    t = 2;
  }
  if(distVal != t){
    distVal = t;
    Serial.print("PS13:");
    Serial.print(t );
    Serial.print(",");
  }


  delay(10);
  while(Serial.available()){
    char c = Serial.read();
    if(c == 'P'){
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
    } 

  }
}

