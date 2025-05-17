const int flowPin = 3; 
volatile int pulseCount = 0;
float totalLiters = 0;
unsigned long oldTime = 0;


void setup() {
  Serial.begin(9600);
  pinMode(flowPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(flowPin), countPulse, RISING);
  oldTime = millis();
}


void loop() {
  if (millis() - oldTime > 1000) { 
    float liters = (pulseCount / 7.5) / 60.0;  
    totalLiters += liters;
    pulseCount = 0;
    oldTime = millis();
    Serial.print("Total water usage: ");
    Serial.print(totalLiters);
    Serial.println(" L");
  }
}


void countPulse() {
  pulseCount++;
}
