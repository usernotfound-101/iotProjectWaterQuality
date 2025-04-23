#include <WiFi.h>
#include <HTTPClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#define TURBIDITY_PIN 35  // GPIO 35
#define TDS_PIN       33  // GPIO 33

const char* ssid     = "Galaxy A35";
const char* password = "curk0958";

const char* serverName = "http://192.168.61.147:5002/insert";  // Flask server endpoint

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000);  // Initialize NTP client with 60s update interval

float estimatePH(int turbidity, int tds) {
  float normTurb = (turbidity - 2080) / 80.0;
  float normTDS = (tds - 80) / 140.0;
  float pH = 7.5 - normTurb - normTDS;
  return pH;
}

bool isUnsafe(int turbidity, int tds, float ph) {
  return (turbidity > 2100 || tds > 130 || ph < 6.5 || ph > 8.5);
}

String formatTimestamp() {
  unsigned long epochTime = timeClient.getEpochTime();  // Get the epoch time (seconds since 1970)
  
  // Cast epochTime to time_t for localtime
  time_t epoch = (time_t)epochTime;
  
  // Convert epoch time to a struct tm
  struct tm* timeStruct = localtime(&epoch);
  
  // Format the timestamp as "YYYY-MM-DD HH:MM:SS"
  String timestamp = String(timeStruct->tm_year + 1900) + "-" + 
                     (timeStruct->tm_mon + 1 < 10 ? "0" : "") + String(timeStruct->tm_mon + 1) + "-" + 
                     (timeStruct->tm_mday < 10 ? "0" : "") + String(timeStruct->tm_mday) + " " + 
                     (timeStruct->tm_hour < 10 ? "0" : "") + String(timeStruct->tm_hour) + ":" + 
                     (timeStruct->tm_min < 10 ? "0" : "") + String(timeStruct->tm_min) + ":" + 
                     (timeStruct->tm_sec < 10 ? "0" : "") + String(timeStruct->tm_sec);

  return timestamp;
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi!");

  // Start the NTP client to sync time
  timeClient.begin();
  while (!timeClient.update()) {
    timeClient.forceUpdate();
  }
}

void loop() {
  int turbidityRaw = analogRead(TURBIDITY_PIN);
  int tdsRaw = analogRead(TDS_PIN);
  float estimatedPH = estimatePH(turbidityRaw, tdsRaw);

  bool unsafe = isUnsafe(turbidityRaw, tdsRaw, estimatedPH);

  String timestamp = formatTimestamp();  // Get the current timestamp

  Serial.print(" | Raw Turbidity: ");
  Serial.print(turbidityRaw);
  Serial.print(" | Raw TDS: ");
  Serial.print(tdsRaw);
  Serial.print(" | Estimated pH: ");
  Serial.print(estimatedPH, 2);
  Serial.print(" | Status: ");
  Serial.println(unsafe ? "WATER UNSAFE" : "WATER SAFE");
  Serial.print(" | Timestamp: ");
  Serial.println(timestamp);

  // Send data to Flask server
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverName);
    http.addHeader("Content-Type", "application/json");

    String postData = "{\"TDS\": " + String(tdsRaw) +
                      ", \"TURBIDITY\": " + String(turbidityRaw) +
                      ", \"PH\": " + String(estimatedPH, 2) +
                      ", \"SAFETY\": \"" + (unsafe ? "WATER UNSAFE" : "WATER SAFE") + "\"" +
                      ", \"TIMESTAMP\": \"" + timestamp + "\"}";

    int httpResponseCode = http.POST(postData);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Server Response: " + response);
    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  }

  delay(5000);
}