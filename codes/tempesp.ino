#include <OneWire.h>
#include <DallasTemperature.h>


#define ONE_WIRE_BUS 2


OneWire oneWire(ONE_WIRE_BUS);


DallasTemperature sensors(&oneWire);

void setup() {
  

  Serial.begin(115200);
  Serial.println("Dallas Temperature Sensor Test");


  sensors.begin();
}

void loop() {
  
  sensors.requestTemperatures();


  float temperatureC = sensors.getTempCByIndex(0);
  Serial.print("Temperature: ");
  Serial.print(temperatureC);
  Serial.println(" °C");

 
  float temperatureF = sensors.toFahrenheit(temperatureC);
  Serial.print("Temperature: ");
  Serial.print(temperatureF);
  Serial.println(" °F");

  
  delay(1000);
}
