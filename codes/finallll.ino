#include <OneWire.h>
#include <DallasTemperature.h>


// ** TDS Sensor Configuration **
#define TdsSensorPin A1
#define VREF 5.0               
#define SCOUNT 30       

int analogBuffer[SCOUNT]
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0;

float averageVoltage = 0;
float tdsValue = 0;
float compensationCoefficient = 1.0;


// ** Temperature Probe Configuration **
#define ONE_WIRE_BUS 2        
OneWire oneWire(ONE_WIRE_BUS);  
DallasTemperature sensors(&oneWire);
float temperature = 25.0;      


// ** Flow Sensor Configuration **
const int flowPin = 3;     
volatile int pulseCount = 0;
float totalLiters = 0;
unsigned long oldTime = 0;


void setup() {
  // ** TDS Sensor Setup **
  Serial.begin(115200);
  pinMode(TdsSensorPin, INPUT);


  // ** Temperature Probe Setup **
  sensors.begin();


  // ** Flow Sensor Setup **
  pinMode(flowPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(flowPin), countPulse, RISING);
  oldTime = millis();
}


void loop() {
  // ** TDS Sensor Measurement **
  static unsigned long analogSampleTimepoint = millis();
  if (millis() - analogSampleTimepoint > 40U) {
    analogSampleTimepoint = millis();
    analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin); 
    analogBufferIndex++;
    if (analogBufferIndex == SCOUNT) {
      analogBufferIndex = 0;
    }
  }
  

  if (millis() - oldTime > 1000) { // Print every second
    oldTime = millis();
    

  
    // Median filtering algorithm for TDS readings
    for (int copyIndex = 0; copyIndex < SCOUNT; copyIndex++) {
      analogBufferTemp[copyIndex] = analogBuffer[copyIndex];
    }
    averageVoltage = getMedianNum(analogBufferTemp, SCOUNT) * (float)VREF / 1024.0;

    // Temperature Compensation
    sensors.requestTemperatures();
    temperature = sensors.getTempCByIndex(0);
    compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0);


    // TDS Calculation
    tdsValue = (133.42 * averageVoltage * averageVoltage * averageVoltage - 
                255.86 * averageVoltage * averageVoltage + 
                857.39 * averageVoltage) * 0.5 * compensationCoefficient;


    // ** Flow Sensor Measurement **
    float liters = (pulseCount / 7.5) / 60.0;
    totalLiters += liters;
    pulseCount = 0; 


    // Send data to ESP via Serial
    Serial.print(tdsValue);
    Serial.print(",");
    Serial.print(temperature);
    Serial.print(",");
    Serial.println(totalLiters);
  }
}


// Median filtering function for TDS sensor
int getMedianNum(int bArray[], int iFilterLen) {
  int bTab[iFilterLen];
  for (byte i = 0; i < iFilterLen; i++)
    bTab[i] = bArray[i];
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
  if ((iFilterLen & 1) > 0)
    bTemp = bTab[(iFilterLen - 1) / 2];
  else
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
  return bTemp;
}

// Flow sensor pulse count ISR
void countPulse() {
  pulseCount++;
}
