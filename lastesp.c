#include <LoRa.h>
#include <SPI.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Constants for LoRa configuration
#define LORA_SS 5
#define LORA_RST 14
#define LORA_DIO0 2
#define LORA_FREQUENCY 433E6

byte localAddress = 0xC0;
byte destinationAddress = 0xDE;
long lastSendTime = 0;
int interval = 5000;
int count = 0;
byte msgCount = 0;            // count of outgoing messages

// Constants for Wi-Fi configuration
const char *ssid = "sop ikan";
const char *password = "tahubulat1234";
const char *serverBaseUrl = "https://delly.biz.id/";

// Endpoints for POST and GET requests
const char *postEndpoint = "api/mon-data";
const char *getEndpoint = "perintahonoffdevice/2020";

void setup() {
  Serial.begin(9600);
  while (!Serial);
  Serial.println("LoRa Sender");
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);

  while (!LoRa.begin(433E6)) {
    Serial.println(".");
    delay(500);
  }

  LoRa.setSyncWord(0xF3);
  Serial.println("LoRa Initialized");

  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void loop() {
  if (millis() - lastSendTime > interval) {
    String sensorData = String(count++);
    sendMessage(sensorData);

    Serial.print("Sending data " + sensorData);
    lastSendTime = millis();
    interval = random(2000) + 1000;
  }

  receiveMessage(LoRa.parsePacket());
}

void sendMessage(String outgoing) {
  HTTPClient http;
  String getUrl = String(serverBaseUrl) + getEndpoint;
  http.begin(getUrl);

  int httpCodeGet = http.GET();

  if (httpCodeGet > 0) {
    String payload = http.getString();
    Serial.println("Response (GET): " + payload);

    DynamicJsonDocument doc(256); // Adjust this capacity according to your JSON response size

    DeserializationError error = deserializeJson(doc, payload);

    if (error) {
      Serial.print("Failed to parse JSON: ");
      Serial.println(error.c_str());
      return;
    }

    const char *status = doc["status"];
    const char *device_id = doc["device_id"];

    String dataKirim = "id:" + String(device_id) + "|status:" + String(status);
    Serial.println(dataKirim);
    LoRa.beginPacket();
    LoRa.write(destinationAddress);
    LoRa.write(localAddress);
    LoRa.write(msgCount);
    LoRa.write(outgoing.length());
    LoRa.print(dataKirim);
    LoRa.endPacket();
    msgCount++;
  } else {
    Serial.println("HTTP GET request failed");
  }

  http.end();
}

void receiveMessage(int packetSize) {
  if (packetSize == 0) return;

  int recipient = LoRa.read();
  byte sender = LoRa.read();
  byte incomingLength = LoRa.read();
  byte incomingMsgId = LoRa.read();

  String incoming = "";

  while (LoRa.available()) {
    incoming += (char)LoRa.read();
  }

  Serial.print("Received data " + incoming);

  int colonIndex = incoming.indexOf(':');
  int vPipeIndex = incoming.indexOf('|', colonIndex);
  int iPipeIndex = incoming.indexOf('|', vPipeIndex + 1);
  int pPipeIndex = incoming.indexOf('|', iPipeIndex + 1);
  int ePipeIndex = incoming.indexOf('|', pPipeIndex + 1);

  String id = incoming.substring(colonIndex + 1, vPipeIndex);
  String voltage = incoming.substring(vPipeIndex + 3, iPipeIndex);
  String current = incoming.substring(iPipeIndex + 3, pPipeIndex);
  String power = incoming.substring(pPipeIndex + 3, ePipeIndex);
  String energy = incoming.substring(ePipeIndex + 3);

  float id1 = id.toFloat();
  float voltage1 = voltage.toFloat();
  float current1 = current.toFloat();
  float power1 = power.toFloat();
  float energy1 = energy.toFloat();

  Serial.print(String(id1) + "|" + String(voltage1) + "|" + String(current1));

  HTTPClient http;
  String postUrl = String(serverBaseUrl) + postEndpoint;
  http.begin(postUrl);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  String postData = "device_id=" + String(id1) + "&voltage=" + String(voltage1) + "&current=" + String(current1) + "&power=" + String(power1) + "&energy=" + String(energy1);

  int httpResponseCode = http.POST(postData);

  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code (POST): ");
    Serial.println(httpResponseCode);
    String response = http.getString();
    Serial.println(postData);
  } else {
    Serial.print("HTTP POST request failed, error code: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}
