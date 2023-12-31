#include <SPI.h>
#include <LoRa.h>

const int csPin = 7;
const int resetPin = 6;
const int irqPin = 1;

byte localAddress = 0xAA;
byte destinationAddress = 0xBB;
long lastSendTime = 0;
int interval = 2000;
int count = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial)
    ;
  Serial.println("LoRa Sender");

  while (!LoRa.begin(433E6))  //433E6 - Asia, 866E6 - Europe, 915E6 - North America
  {
    Serial.println(".");
  }

  Serial.println("LoRa Initializing OK!");
}
void loop() {
  if (millis() - lastSendTime > interval) {
    String sensorData = String(count++);
    sendMessage(sensorData);

    Serial.print("Sending data ");
    Serial.print(" from 0x" + String(localAddress, HEX));
    Serial.println(" to 0x" + String(destinationAddress, HEX));

    lastSendTime = millis();
    interval = random(2000) + 1000;
  }

  receiveMessage(LoRa.parsePacket());
}

void sendMessage(String outgoing) {
  float voltage = rand() % 100;
  float current = rand() % 100;
  float energy = rand() % 100;
  float power = rand() % 100;

  float device_id = 2910;

  String data = "id:" + String(device_id) + "|V:" + String(voltage) + "|I:" + String(current) + "|P:" + String(power) + "|E:" + String(energy);
  LoRa.beginPacket();
  LoRa.write(destinationAddress);
  LoRa.write(localAddress);
  LoRa.write(outgoing.length());
  LoRa.print(data);
  LoRa.endPacket();
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
  // Find "id:" and "status:" in the received packet
  int idPosition = incoming.indexOf("id:");
  int statusPosition = incoming.indexOf("status:");

  // Check if "id:" and "status:" are found
  if (idPosition != -1 && statusPosition != -1) {
    // Extract id and status
    int idEnd = incoming.indexOf('|', idPosition);  // Find the position of '|' after "id:"
    if (idEnd != -1) {
      String id = incoming.substring(idPosition + 3, idEnd);
      String status = incoming.substring(statusPosition + 7);  // Length of "status:" is 7

      // Print the extracted values
      Serial.print("ID: ");
      Serial.println(id);
      Serial.print("Status: ");
      Serial.println(status);

      if (status == "mati") {
        Serial.println("ledmati");
      }
      if (status == "hidup"){
        Serial.println("ledhidup");
      }
    } else {
      // If '|' is not found after "id:", print an error message
      Serial.println("Invalid packet format - Missing '|'");
    }
  } else {
    // If "id:" or "status:" is not found, print an error message
    Serial.println("ID or Status not found in the packet");
  }



  // Clear receivedData for the next packet
  incoming = "";
}