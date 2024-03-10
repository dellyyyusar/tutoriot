#include <SPI.h>
#include <LoRa.h>
#include <PZEM004Tv30.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

const int pinRelayOut = 5;
const int pinRelayDis = 6;
const int pinRelayCon = 7;

PZEM004Tv30 pzem(3, 4);
byte msgCount = 0;            // count of outgoing messages

long lastSendTime = 0;
int interval = 5000;
int count = 0;

byte localAddress = 0xDE;
byte destinationAddress = 0xC0;

void setup() {
  pinMode(pinRelayOut, OUTPUT);
  pinMode(pinRelayDis, OUTPUT);
  pinMode(pinRelayCon, OUTPUT);
  digitalWrite(pinRelayOut, LOW);
  digitalWrite(pinRelayCon, LOW);
  digitalWrite(pinRelayDis, LOW);

  Serial.begin(9600);
  while (!Serial);
  Serial.println("LoRa Sender");

  if (!LoRa.begin(433E6)) { // Ubah frekuensi LoRa sesuai dengan pengaturan perangkat Anda
    Serial.println("LoRa initialization failed. Check your connections.");
    while (true);
  }

  LoRa.setSyncWord(0xF3);
  Serial.println("LoRa Initializing OK!");
}

void loop() {
  if (millis() - lastSendTime > interval) {
    String sensorData = String(count++);
    sendMessage(sensorData);

    lastSendTime = millis();
  }

  receiveMessage(LoRa.parsePacket());
}

void sendMessage(String outgoing) {
  float voltage = pzem.voltage();
  float current = pzem.current();
  float power = pzem.power();
  float energy = pzem.energy();

  float device_id = 2020;

  String data = "id:" + String(device_id) + "|V:" + String(voltage) + "|I:" + String(current) + "|P:" + String(power) + "|E:" + String(energy);
  Serial.println("Sending data: " + data);

  LoRa.beginPacket();
  LoRa.write(destinationAddress);
    LoRa.write(localAddress);
    LoRa.write(msgCount);
    LoRa.write(outgoing.length());
  LoRa.print(data);
  LoRa.endPacket();
    msgCount++;

}

void receiveMessage(int packetSize) {
  if (packetSize > 0) {
    String incoming = "";

    while (LoRa.available()) {
      incoming += (char)LoRa.read();
    }

    int idPosition = incoming.indexOf("id:");
    int statusPosition = incoming.indexOf("status:");

    if (idPosition != -1 && statusPosition != -1) {
      int idEnd = incoming.indexOf('|', idPosition);
      if (idEnd != -1) {
        String id = incoming.substring(idPosition + 3, idEnd);
        String status = incoming.substring(statusPosition + 7);

        Serial.print("ID: ");
        Serial.println(id);
        Serial.print("Status: ");
        Serial.println(status);

        // Update relay status based on received status
        if (status.equals("mati")) {
          Serial.println("Relay dimatikan");
          digitalWrite(pinRelayOut, LOW);
        } else if (status.equals("hidup")) {
          Serial.println("Relay dihidupkan");
          digitalWrite(pinRelayOut, HIGH);
          digitalWrite(pinRelayCon, HIGH);
        } else {
          Serial.println("Status tidak valid");
        }
      } else {
        Serial.println("Format paket tidak valid - Kurang '|'");
      }
    } else {
      Serial.println("ID atau Status tidak ditemukan dalam paket");
    }
  }
}
