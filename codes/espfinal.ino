#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>

// WiFi credentials
const char* ssid = "your_wifi_name";
const char* password = "Your_wifi_pass";


// ThingSpeak API URL (Replace with your own API key)
String URL = "https://api.thingspeak.com/update?api_key=YJ9UM2ENSDU563LS&field1=";

void setup() {
  Serial.begin(115200);  // Communication with Arduino
  Serial.println("Booting up...");
  
  WiFi.disconnect();
  delay(2000);
  
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  // Connecting to WiFi
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi.");
  Serial.println("IP Address: " + WiFi.localIP().toString());
}

void loop() {
  if (Serial.available() > 0) {
    // Read the data from Arduino
    String data = Serial.readStringUntil('\n');
    Serial.println("Received: " + data);

    // Parse the sensor data (assuming the format: "TDS:xx,TempProbe:xx,WaterUsed:xx")
    float tds = parseData(data, "TDS:");
    float tempProbe = parseData(data, "TempProbe:");
    float waterUsed = parseData(data, "WaterUsed:");

    // Print parsed data for debugging
    Serial.println("Parsed Data - TDS: " + String(tds) + ", TempProbe: " + String(tempProbe) + ", WaterUsed: " + String(waterUsed));

    // Send the data to ThingSpeak
    sendDataToThingSpeak(tds, tempProbe, waterUsed);
  }
}

float parseData(String data, String key) {
  int keyIndex = data.indexOf(key);
  int separatorIndex = data.indexOf(',', keyIndex);
  if (keyIndex == -1 || separatorIndex == -1) {
    Serial.println("Parsing Error: " + key);
    return 0; // Return zero if parsing failed
  }
  return data.substring(keyIndex + key.length(), separatorIndex).toFloat();
}

void sendDataToThingSpeak(float tds, float tempProbe, float waterUsed) {
  WiFiClient client;
  HTTPClient http;

  String newUrl = URL + "&field1=" + String(tds) + "&field2=" + String(tempProbe) + "&field3=" + String(waterUsed);
  Serial.println("Sending to ThingSpeak: " + newUrl);
  
  http.begin(client, newUrl);
  int responseCode = http.GET();
  String responseData = http.getString();

  Serial.println("Response Code: " + String(responseCode));
  Serial.println("Response Data: " + responseData);
  http.end();
}
