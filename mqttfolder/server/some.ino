#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "Haaris's A35";
const char* password = "ArsenalItsNotJustAArsenal";

const char* server = "http://api.thingspeak.com/update";
const char* apiKey = "1LQOPPC0IODLQTRO";

#define TDS_PIN 32  

void setup() {
  Serial.begin(115200);
  delay(1000);

  analogReadResolution(12); // 12-bit resolution (0-4095)

  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
}

void loop() {
  int tdsRaw = 0;
  for (int i = 0; i < 10; i++) { 
    tdsRaw += analogRead(TDS_PIN);
    delay(10);
  }
  tdsRaw /= 10;
  Serial.print("Raw TDS Value: ");
  Serial.println(tdsRaw);

  int tdsToSend = tdsRaw;
  Serial.println(tdsToSend);

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = String(server) + "?api_key=" + apiKey + "&field1=" + String(tdsToSend);
    
    http.begin(url);
    int responseCode = http.GET();
    
    if (responseCode > 0) {
      Serial.println("Data sent! Response Code: " + String(responseCode));
      String response = http.getString();
    } else {
      Serial.println("Error sending data: " + http.errorToString(responseCode));
    }
    
    http.end();
  } else {
    Serial.println("WiFi not connected.");
  }

  delay(15000);  
}