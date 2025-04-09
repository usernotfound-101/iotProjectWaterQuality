#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "Haaris's A35";
const char* password = "ArsenalItsNotJustAArsenal";

// ThingSpeak
const char* tsServer = "http://api.thingspeak.com/update";
const char* apiKey = "1LQOPPC0IODLQTRO";

// Flask server (change IP to your PC's IP)
const char* flaskServer = "http://192.168.1.100:5002/insert";  // <-- Replace with your Flask server IP

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
  randomSeed(analogRead(0));
}

void loop() {
  // -------- TDS Sensor Reading --------
  int tdsRaw = 0;
  for (int i = 0; i < 10; i++) { 
    tdsRaw += analogRead(TDS_PIN);
    delay(10);
  }
  tdsRaw /= 10;

  // -------- Simulated Readings --------
  int pH = random(700, 901);            // Random pH (700-900)
  int turbidity = random(1200, 1601);   // Random Turbidity (1200-1600)

  // -------- Debug Output --------
  Serial.print("TDS: "); Serial.println(tdsRaw);
  Serial.print("pH: "); Serial.println(pH);
  Serial.print("Turbidity: "); Serial.println(turbidity);

  // -------- Send to ThingSpeak (TDS only) --------
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String tsUrl = String(tsServer) + "?api_key=" + apiKey + "&field1=" + String(tdsRaw);
    
    http.begin(tsUrl);
    int responseCode = http.GET();
    
    if (responseCode > 0) {
      Serial.println("ThingSpeak Response: " + String(responseCode));
    } else {
      Serial.println("ThingSpeak Error: " + http.errorToString(responseCode));
    }
    http.end();
  }

  // -------- Send to Flask Server (All Data) --------
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(flaskServer);
    http.addHeader("Content-Type", "application/json");

    StaticJsonDocument<200> doc;
    doc["TDS"] = tdsRaw;
    doc["PH"] = pH;
    doc["TURBIDITY"] = turbidity;

    String jsonData;
    serializeJson(doc, jsonData);

    int httpResponseCode = http.POST(jsonData);

    if (httpResponseCode > 0) {
      Serial.println("Flask POST success: " + String(httpResponseCode));
    } else {
      Serial.println("Flask POST error: " + http.errorToString(httpResponseCode));
    }

    http.end();
  }

  delay(5000);  // 15s delay (ThingSpeak rate limit)
}
