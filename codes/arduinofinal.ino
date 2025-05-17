#include <OneWire.h>
#include <DallasTemperature.h>

// Define pins and constants
#define ONE_WIRE_BUS 2         
#define TdsSensorPin A0         
#define FLOW_SENSOR_PIN 3       
#define RELAY_PIN 8            
#define TRIG_PIN 6              
#define ECHO_PIN 7              
#define VREF 5.0               
#define SCOUNT 30        

// Variables
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

int analogBuffer[SCOUNT];
int analogBufferIndex = 0;
float tdsValue = 0;
volatile int pulseCount = 0;
float totalLiters = 0;
unsigned long oldTime = 0;

void setup() {
  Serial.begin(115200); 
  pinMode(TdsSensorPin, INPUT);
  pinMode(FLOW_SENSOR_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  digitalWrite(RELAY_PIN, LOW);  

  attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), countPulse, RISING);


  sensors.begin();
}

void loop() {
  // Get DS18B20 temperature
  sensors.requestTemperatures();
  float tempProbe = sensors.getTempCByIndex(0);

  // Calculate TDS
  analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin);
  analogBufferIndex++;
  if (analogBufferIndex == SCOUNT) {
    analogBufferIndex = 0;
    float averageVoltage = getMedianNum(analogBuffer, SCOUNT) * (float)VREF / 1024.0;
    tdsValue = (133.42 * averageVoltage * averageVoltage * averageVoltage - 255.86 * averageVoltage * averageVoltage + 857.39 * averageVoltage) * 0.5;
  }

  // Check water flow every second
  unsigned long currentTime = millis();
  if (currentTime - oldTime > 1000) {
    float liters = (pulseCount / 7.5) / 60;
    totalLiters += liters;
    pulseCount = 0;
    oldTime = currentTime;
  }

  // Ultrasonic sensor to control pump
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH);
  float distance = (duration * 0.034) / 2;

  if (distance < 10) {
    digitalWrite(RELAY_PIN, HIGH);
  } else if (distance > 50) {
    digitalWrite(RELAY_PIN, LOW); 
  }

  // Transmit data to ESP8266
  String data = "TDS:" + String(tdsValue) + ",TempProbe:" + String(tempProbe) + ",WaterUsed:" + String(totalLiters);
  Serial.println(data);
  delay(2000);
}

// Helper function for median calculation
int getMedianNum(int bArray[], int iFilterLen) {
  int bTab[iFilterLen];
  for (byte i = 0; i < iFilterLen; i++) bTab[i] = bArray[i];
  int i, j, bTemp;
  for (j = 0; j < iFilterLen - 1; j++) {
    for (i = 0; i < iFilterLen - j - 1; i++) {
      if (bTab[i] > bTab[i + 1]) {
        bTemp = bTab[i];
        bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
      }
    }
  }
  return (iFilterLen & 1) ? bTab[(iFilterLen - 1) / 2] : (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
}

// ISR for flow sensor
void countPulse() {
  pulseCount++;
}
