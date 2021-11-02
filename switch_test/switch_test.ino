const int buzzer = 3; 
const int sensor = 4;

int state; // 0 close - 1 open wwitch

void setup()
{
  Serial.begin(9600);
  pinMode(sensor, INPUT_PULLUP);
}

void loop()
{
  state = digitalRead(sensor);
  
  if (state == HIGH){
    Serial.println("Open");
    //tone(buzzer, 50);
  }
  else{
    Serial.println("Closed");
    //noTone(buzzer);
  }
  delay(200);
}
