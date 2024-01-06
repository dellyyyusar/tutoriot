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

byte localAddress = 0xBB;
byte destinationAddress = 0xAA;
long lastSendTime = 0;
int interval = 5000;
int count = 0;

// Variables for sensor data
float id1 = 0.00;
float voltage1 = 0.00;
float current1 = 0.00;
float power1 = 0.00;
float energy1 = 0.00;

// Constants for Wi-Fi configuration
const char *ssid = "sop ikan";
const char *password = "tahubulat1234";
const char *serverBaseUrl = "http://192.168.1.6:8000/";

// Endpoints for POST and GET requests
const char *postEndpoint = "api/mon-data";
const char *getEndpoint = "perintahonoffdevice/2020";


void setup() {
  Serial.begin(9600);
  while (!Serial)
    ;
  Serial.println("LoRa Sender");
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);

  while (!LoRa.begin(433E6))  //433E6 - Asia, 866E6 - Europe, 915E6 - North America
  {
    Serial.println(".");
  }

  Serial.println("LoRa Initializing OK!");

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
    Serial.print(" from source 0x" + String(localAddress, HEX));
    Serial.println(" to destination 0x" + String(destinationAddress, HEX));

    lastSendTime = millis();
    interval = random(2000) + 1000;
  }

  receiveMessage(LoRa.parsePacket());
}

void sendMessage(String outgoing) {
  HTTPClient http;
  String getUrl = String(serverBaseUrl) + getEndpoint;  // Use String objects
  http.begin(getUrl);

  int httpCodeGet = http.GET();


  if (httpCodeGet > 0) {
    String payload = http.getString();
    Serial.println("Response (GET): " + payload);
    // Data JSON yang akan di-parse
    String jsonString = payload;

    // Ubah String ke const char*
    const char *json = jsonString.c_str();

    // Ukuran buffer yang cukup besar untuk menyimpan struktur JSON
    const size_t capacity = JSON_OBJECT_SIZE(2) + 40;
    DynamicJsonDocument doc(capacity);

    // Parse JSON
    DeserializationError error = deserializeJson(doc, json);

    // Cek apakah parsing berhasil
    if (error) {
      Serial.print(F("Gagal parsing JSON! Error code: "));
      Serial.println(error.c_str());
      return;
    }

    // Ambil nilai dari JSON
    const char *status = doc["status"];
    const char *device_id = doc["device_id"];

    String dataKirim = "id:" + String(device_id) + "|status:" + String(status);
    LoRa.beginPacket();
    LoRa.write(destinationAddress);
    LoRa.write(localAddress);
    LoRa.write(outgoing.length());
    LoRa.print(dataKirim);
    LoRa.endPacket();

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

  String incoming = "";

  while (LoRa.available()) {
    incoming += (char)LoRa.read();
  }

  // if (incomingLength != incoming.length()) {
  //   Serial.println("Error: Message length does not match length");
  //   return;
  // }

  if (recipient != localAddress) {
    Serial.println("Error: Recipient address does not match local address");
    return;
  }

  Serial.print("Received data " + incoming);
  Serial.print(" from 0x" + String(sender, HEX));
  Serial.println(" to 0x" + String(recipient, HEX));
  // Parse and process your received data here
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

  // Konversi string menjadi float
  id1 = id.toFloat();
  voltage1 = voltage.toFloat();
  current1 = current.toFloat();
  power1 = power.toFloat();
  energy1 = energy.toFloat();
  Serial.print(String(id1) + "|" + String(voltage1) + "|" + String(current1));
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
    Serial.println(postData);
  } else {
    Serial.print("HTTP POST request failed, error code: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}