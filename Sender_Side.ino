#include <SPI.h>
#include <LoRa.h>

#define SS 5
#define RST 14
#define DIO0 2

#define SOIL_SENSOR_PIN 34
#define RELAY_PIN 12  // Relay control pin

#define DRY_VALUE 200     // Sensor value when soil is completely dry
#define WET_VALUE 2500    // Sensor value when soil is fully wet

unsigned long lastActionTime = 0;
bool isSending = true;

void setup() {
    Serial.begin(115200);
    while (!Serial);

    Serial.println("🚀 ESP32 LoRa Sender Booting Up...");

    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW);  // Start with relay OFF

    LoRa.setPins(SS, RST, DIO0);
    
    if (!LoRa.begin(865E6)) {
        Serial.println("❌ LoRa Init FAILED! Check wiring!");
        while (1);
    }

    Serial.println("✅ LoRa Ready!");
    LoRa.setSpreadingFactor(7);
    LoRa.setSignalBandwidth(125E3);
    LoRa.setCodingRate4(5);
    LoRa.enableCrc();

    lastActionTime = millis();
}

void loop() {
    unsigned long currentMillis = millis();

    if (isSending) {
        if (currentMillis - lastActionTime < 3000) {
            sendSoilMoisture();
            delay(500); // small delay to avoid flooding
        } else {
            isSending = false;
            lastActionTime = currentMillis;
            Serial.println("🛑 Done sending. Switching to LISTEN mode.");
        }
    } else {
        if (currentMillis - lastActionTime < 3000) {
            checkIncomingLoRa();
        } else {
            isSending = true;
            lastActionTime = currentMillis;
            Serial.println("📤 Switching back to SEND mode.");
        }
    }
}

void checkIncomingLoRa() {
    Serial.println("👂 Listening for LoRa messages...");
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
        String incomingMsg = "";
        while (LoRa.available()) {
            incomingMsg += (char)LoRa.read();
        }

        incomingMsg.trim();
        Serial.print("📥 LoRa Received: ");
        Serial.println(incomingMsg);

        // Ignore own messages (soil data prefixed with "S:")
        if (incomingMsg.startsWith("S:")) {
            Serial.println("🚫 Ignoring own message.");
            return;
        }

        if (incomingMsg.equalsIgnoreCase("ON")) {
            digitalWrite(RELAY_PIN, HIGH);
            Serial.println("⚡ Relay TURNED ON via LoRa!");
        } else if (incomingMsg.equalsIgnoreCase("OFF")) {
            digitalWrite(RELAY_PIN, LOW);
            Serial.println("🔌 Relay TURNED OFF via LoRa!");
        } else {
            Serial.println("🤷 Unknown LoRa command.");
        }
    }
}

void sendSoilMoisture() {
    int rawValue = analogRead(SOIL_SENSOR_PIN);
    Serial.print("🌱 Soil Raw: ");
    Serial.println(rawValue);

    int moisturePercent = map(rawValue, WET_VALUE, DRY_VALUE, 100, 0);  
    moisturePercent = constrain(moisturePercent, 0, 100);

    Serial.print("📡 Sending Moisture: ");
    Serial.print(moisturePercent);
    Serial.println("%");

    String message = "S:" + String(moisturePercent);

    LoRa.beginPacket();
    LoRa.print(message);
    LoRa.endPacket();
}
