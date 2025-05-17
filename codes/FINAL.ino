////////-------->website Link for data Represntation  https://water-ivory.vercel.app/  <------------//////////

#include <ESP8266WiFi.h>

// ** WiFi and ThingSpeak Configuration **
const char* ssid = "";                 // Enter your WiFi SSID
const char* password = "";                      // Enter your WiFi Password
const char* server = "";              // ThingSpeak server address
const char* apiKey = "";                // Your ThingSpeak API Key

WiFiClient client;  // Create a WiFiClient object

// Variables for sensor data
float tdsValue = 0.0;
float temperature = 0.0;
float totalLiters = 0.0;

void setup() {
  Serial.begin(115200);
  
  // Connect to WiFi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
}

void loop() {
  // Read data from Arduino
  if (Serial.available()) {
    String data = Serial.readStringUntil('\n');
    sscanf(data.c_str(), "%f,%f,%f", &tdsValue, &temperature, &totalLiters);

    // Send data to ThingSpeak
    if (client.connect(server, 80)) {  // Connect to ThingSpeak server on port 80
      String postStr = "api_key=" + String(apiKey);
      postStr += "&field1=" + String(tdsValue);
      postStr += "&field2=" + String(temperature);
      postStr += "&field3=" + String(totalLiters);

      client.print("POST /update HTTP/1.1\n");
      client.print("Host: " + String(server) + "\n");
      client.print("Connection: close\n");
      client.print("Content-Type: application/x-www-form-urlencoded\n");
      client.print("Content-Length: " + String(postStr.length()) + "\n\n");
      client.print(postStr);

      Serial.println("Data sent to ThingSpeak:");
      Serial.println(postStr);
      
      client.stop(); // Close the connection
    }
  }

  // Wait for 15 seconds to send data again
  delay(15000);
}
