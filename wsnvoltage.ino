#include <LoRa.h>
#include <SPI.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define ss 5
#define rst 14
#define dio0 2

const char *ssid = "sop ikan";
const char *password = "tahubulat1234";
const char *serverUrl = "http://delly.biz.id/api/mon-data";


int counter = 0;

// float id = 0.00;
// float voltage = 0.00;
// float current = 0.00;
// float power = 0.00;
// float energy = 0.00;
// String id, voltage, current, power, energy;
  float id1 = 0.00;
  float voltage1 = 0.00;
  float current1 = 0.00;
  float power1 = 0.00;
  float energy1 = 0.00;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.println("LoRa Sender");

  LoRa.setPins(ss, rst, dio0);  //setup LoRa transceiver module

  while (!LoRa.begin(433E6))  //433E6 - Asia, 866E6 - Europe, 915E6 - North America
  {
    Serial.println(".");
    delay(500);
  }
  LoRa.setSyncWord(0xA5);
  Serial.println("LoRa Initializing OK!");

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void loop() {

  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    while (LoRa.available()) {
      String receivedData = LoRa.readString();
      Serial.println("Received data: " + receivedData);

      // cari delimiter
      String id, voltage, current, power, energy;
      int colonIndex = receivedData.indexOf(':');
      int vPipeIndex = receivedData.indexOf('|', colonIndex);
      int iPipeIndex = receivedData.indexOf('|', vPipeIndex + 1);
      int pPipeIndex = receivedData.indexOf('|', iPipeIndex + 1);
      int ePipeIndex = receivedData.indexOf('|', pPipeIndex + 1);
      // ekstrak subdata
      id = receivedData.substring(colonIndex + 1, vPipeIndex);
      voltage = receivedData.substring(vPipeIndex + 3, iPipeIndex);
      current = receivedData.substring(iPipeIndex + 3, pPipeIndex);
      power = receivedData.substring(pPipeIndex + 3, ePipeIndex);
      energy = receivedData.substring(ePipeIndex + 3);
      // print data
      Serial.print("ID: ");
      Serial.println(id);
      Serial.print("Voltage: ");
      Serial.println(voltage);
      Serial.print("Current: ");
      Serial.println(current);
      Serial.print("Power: ");
      Serial.println(power);
      Serial.print("Energy: ");
      Serial.println(energy);

      id1 = id.toFloat();
      voltage1 = voltage.toFloat();
      current1 = current.toFloat();
      power1 = power.toFloat();
      energy1 = energy.toFloat();
      
    }
    HTTPClient http;

      // Specify the server endpoint for the POST request
      http.begin(serverUrl);

      // Set content type
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");

      // Prepare the data to be sent
      String postData = "device_id=" + String(id1) +
                    "&voltage=" + String(voltage1) +
                    "&current=" + String(current1) +
                    "&power=" + String(power1) +
                    "&energy=" + String(energy1);

      // Send the POST request
      int httpResponseCode = http.POST(postData);

      // Check for a successful POST request
      if (httpResponseCode > 0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);

        // Get the response from the server
        String response = http.getString();
        Serial.println(response);
      } else {
        Serial.print("HTTP POST request failed, error code: ");
        Serial.println(httpResponseCode);
      }

      // Close connection
      http.end();
  }
}
