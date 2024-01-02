#include <LoRa.h>
#include <SPI.h>
#include <WiFi.h>
#include <HTTPClient.h>

// Constants for LoRa configuration
#define LORA_SS 5
#define LORA_RST 14
#define LORA_DIO0 2
#define LORA_FREQUENCY 433E6

// Constants for Wi-Fi configuration
const char *ssid = "sop ikan";
const char *password = "tahubulat1234";
const char *serverBaseUrl = "http://192.168.1.6:8000/";

// Endpoints for POST and GET requests
const char *postEndpoint = "api/mon-data";
const char *getEndpoint = "perintahonoffdevice/2020";

// Variables for sensor data
float id1 = 0.00;
float voltage1 = 0.00;
float current1 = 0.00;
float power1 = 0.00;
float energy1 = 0.00;

void setupWiFi() {
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void setupLoRa() {
  Serial.println("Initializing LoRa...");
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  while (!LoRa.begin(LORA_FREQUENCY)) {
    Serial.println("Error initializing LoRa. Retrying...");
    delay(500);
  }
  LoRa.setSyncWord(0xA5);
  Serial.println("LoRa Initializing OK!");
}

void sendHttpPostRequest() {
  HTTPClient http;
  String postUrl = String(serverBaseUrl) + postEndpoint;  // Use String objects
  http.begin(postUrl);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  String postData = "device_id=" + String(id1) + "&voltage=" + String(voltage1) + "&current=" + String(current1) + "&power=" + String(power1) + "&energy=" + String(energy1);

  int httpResponseCode = http.POST(postData);

  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code (POST): ");
    Serial.println(httpResponseCode);
    String response = http.getString();
    Serial.println(response);
  } else {
    Serial.print("HTTP POST request failed, error code: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}

void sendHttpGetRequest() {
  HTTPClient http;
  String getUrl = String(serverBaseUrl) + getEndpoint;  // Use String objects
  http.begin(getUrl);

  int httpCodeGet = http.GET();

  if (httpCodeGet > 0) {
    String payload = http.getString();
    Serial.println("Response (GET): " + payload);

    // Process your GET response here

  } else {
    Serial.println("HTTP GET request failed");
  }

  http.end();
}

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.println("LoRa Sender");

  setupLoRa();
  setupWiFi();
}

void loop() {
  int packetSize = LoRa.parsePacket();

  if (packetSize) {
    while (LoRa.available()) {
      String receivedData = LoRa.readString();
      Serial.println("Received data: " + receivedData);

      // Parse and process your received data here

      int colonIndex = receivedData.indexOf(':');
      int vPipeIndex = receivedData.indexOf('|', colonIndex);
      int iPipeIndex = receivedData.indexOf('|', vPipeIndex + 1);
      int pPipeIndex = receivedData.indexOf('|', iPipeIndex + 1);
      int ePipeIndex = receivedData.indexOf('|', pPipeIndex + 1);

      String id = receivedData.substring(colonIndex + 1, vPipeIndex);
      String voltage = receivedData.substring(vPipeIndex + 3, iPipeIndex);
      String current = receivedData.substring(iPipeIndex + 3, pPipeIndex);
      String power = receivedData.substring(pPipeIndex + 3, ePipeIndex);
      String energy = receivedData.substring(ePipeIndex + 3);

      // Konversi string menjadi float
      id1 = id.toFloat();
      voltage1 = voltage.toFloat();
      current1 = current.toFloat();
      power1 = power.toFloat();
      energy1 = energy.toFloat();
    }

    sendHttpPostRequest();
    sendHttpGetRequest();
  }
}
