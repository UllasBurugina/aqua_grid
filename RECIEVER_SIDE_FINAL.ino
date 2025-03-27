// edit the config.h tab and enter your Adafruit IO credentials
// and any additional configuration needed for WiFi, cellular,
// or ethernet clients.
#include "config.h"

#include <Adafruit_Sensor.h>

// LoRa Libraries
#include <SPI.h>
#include <LoRa.h>

//define the pins used by the transceiver module
#define ss 5
#define rst 14
#define dio0 2

String readingID = "";
String m = "";
String t = "";
String h = "";
String p = "";

int timer = 0;
const int relay = 26;

WiFiClient client;

Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, IO_USERNAME, IO_KEY);
// set up the 'temperature', 'humidity, 'moisture' and 'pump' feeds
AdafruitIO_Feed *temperature = io.feed("temperature");
AdafruitIO_Feed *humidity = io.feed("humidity");
AdafruitIO_Feed *moisture = io.feed("moisture");
AdafruitIO_Feed *PUMP = io.feed("pump");
Adafruit_MQTT_Subscribe pump = Adafruit_MQTT_Subscribe(&mqtt, IO_USERNAME "/feeds/pump");

void setup() {

  // start the serial connection
  Serial.begin(115200);

  // wait for serial monitor to open
  while(! Serial);

  pinMode(relay, OUTPUT);
  
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

  // connect to io.adafruit.com
  Serial.print("Connecting to Adafruit IO");
  io.connect();
  // wait for a connection
  while(io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  PUMP->onMessage(pumpRead);
  // we are connected
  Serial.println();
  Serial.println(io.statusText());
  mqtt.subscribe(&pump);
  PUMP->get();
}

void loop() {

  // io.run(); is required for all sketches.
  // it should always be present at the top of your loop
  // function. it keeps the client connected to
  // io.adafruit.com, and processes any incoming data.
  io.run();
  recieveData();
  delay(1000);
  timer++;
  if (timer>6) {
    sendData();
    timer = 0;
  }
}


void pumpRead(AdafruitIO_Data *data) {
  Serial.print("Pump: ");
  if(data->value()[0] == '1')
  {
    Serial.println("  ON");
    p = "on";
    digitalWrite(relay, HIGH);
  }
  else
  {
    Serial.println("  OFF");
    p = "off";
    digitalWrite(relay, LOW);
  }
  int counter = 0;
  while(counter<6) {
    LoRa.beginPacket();
    LoRa.print(p);
    LoRa.endPacket();
    counter++;
    delay(300);
  }
  counter = 0;
}


void recieveData() {
  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet
    Serial.print("Received packet '");

    // read packet
    while (LoRa.available()) {
      String LoRaData = LoRa.readString();
      Serial.println(LoRaData);
      // LoRaData format: readingID/temperature&humidity$moisture@
      // String example: 1/27.00&65$45@
      int pos1 = LoRaData.indexOf('/');
      int pos2 = LoRaData.indexOf('&');
      int pos3 = LoRaData.indexOf('$');
      int pos4 = LoRaData.indexOf('@');
      readingID = LoRaData.substring(0, pos1);
      t = LoRaData.substring(pos1 + 1, pos2);
      h = LoRaData.substring(pos2 + 1, pos3);
      m = LoRaData.substring(pos3 + 1, pos4);
      Serial.println(readingID);
    }
  }
}


void sendData() {
  sensors_event_t event;
  //dht.temperature().getEvent(&event);

  Serial.print("celsius: ");
  Serial.print(t);
  Serial.println("C");

  // save celsius to Adafruit IO
  temperature->save(t);

  //dht.humidity().getEvent(&event);

  Serial.print("humidity: ");
  Serial.print(h);
  Serial.println("%");

  // save humidity to Adafruit IO
  humidity->save(h);

  Serial.print("moisture: ");
  Serial.print(m);
  Serial.println("%");

  // save moisture to Adafruit IO
  moisture->save(m);
}