#include <Wire.h>
#include <PZEM004Tv30.h>
#include <LoRa.h>
#include <SPI.h>
#include <stdio.h>
#include <LiquidCrystal_I2C.h>

PZEM004Tv30 pzem(3, 4);
LiquidCrystal_I2C lcd(0x27, 20, 3);
 
int counter = 0;
 
void setup() 
{
  Serial.begin(115200); 
  while (!Serial);
  Serial.println("LoRa Sender");
  
  while (!LoRa.begin(433E6))     //433E6 - Asia, 866E6 - Europe, 915E6 - North America
  {
    Serial.println(".");
    delay(500);
  }
  LoRa.setSyncWord(0xA5);
  Serial.println("LoRa Initializing OK!");

  lcd.init();  // Initialize the LCD
  lcd.backlight();   // Turn on the backlight
  lcd.print("Hello");
}
 
void loop() 
{
  // float voltage = pzem.voltage();
  // float current = pzem.current();
  // float energy = pzem.energy();
  // float power = pzem.power();

  float voltage = rand() % 100;
  float current = rand() % 100;
  float energy = rand() % 100;
  float power = rand() % 100;

  float device_id = 2910;

  String data = "id:" + String(device_id) + "|V:" + String(voltage) + "|I:" + String(current) + "|P:" + String(power) + "|E:" + String(energy);

  lcd.clear();
  lcd.setCursor(1,0);
  lcd.print("device_id");
  lcd.setCursor(10,0);
  lcd.print(device_id);
  lcd.setCursor(1,2);
  lcd.print("V : ");
  lcd.setCursor(4,2);
  lcd.print(voltage);
  lcd.setCursor(1,1);
  lcd.print("A : ");
  lcd.setCursor(4,1);
  lcd.print(current);
 
  LoRa.beginPacket();   //Send LoRa packet to receiver
  LoRa.print(data);
  LoRa.endPacket();

  Serial.println("Data sent: " + data);
 
  delay(10000);
}