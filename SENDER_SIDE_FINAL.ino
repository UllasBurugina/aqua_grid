#include <DHT.h>
#include <DHT_U.h>

// LoRa Libraries
#include <SPI.h>
#include <LoRa.h>

//define the pins used by the transceiver module
#define ss 5
#define rst 14
#define dio0 2

int readingID = 0;
String m = "12";
String t = "3";
String h = "42";
String p = "";

int counter = 0;
int timer = 0;
String LoRaMessage = "";
const int moisture_sensor = 4;
const int relay = 26;

// pin connected to DH22 data line
#define DATA_PIN 2

// create DHT22 instance
DHT_Unified dht(DATA_PIN, DHT22);

void setup() {

  // start the serial connection
  Serial.begin(115200);

  //setup LoRa transceiver module
  LoRa.setPins(ss, rst, dio0);

  // 433E6 for ASIA
  while (!LoRa.begin(433E6)) {
    Serial.println(".");
    delay(500);
  }  

  // Change sync word (0xF3) to match the receiver
  // The sync word assures you don't get LoRa messages from other LoRa transceivers
  // ranges from 0-0xFF
  // LoRa.setSyncWord(0xF3);
  Serial.println("LoRa Initializing OK!");
}

void loop() {
  sendReading();
  delay(500);
}


void sendReading() {
  LoRaMessage = "hello";
  Serial.println(LoRaMessage);
  LoRa.beginPacket();
  LoRa.print(LoRaMessage);
  LoRa.endPacket();
}
