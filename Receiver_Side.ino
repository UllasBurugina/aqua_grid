#include <SPI.h>
#include <LoRa.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define SS 5     
#define RST 14   
#define DIO0 2   

int tdm = 1;

const char* ssid = "Oreo_2.4G";
const char* password = "Bangalore1";

const char* serverName = "https://script.google.com/macros/s/AKfycbyVlQNkdvjVdyYt4qXZ18Ut_EsulfPWpvYW0reTVboxcdqW7ssFHL_oIjPCRU20K1wM/exec";

String sheetValue = "";  // Value from Google Sheets

void setup() {
    Serial.begin(9600);
    while (!Serial);

    Serial.println("🚀 Transmitter ESP32 Starting...");
    WiFi.begin(ssid, password);
    Serial.print("🔌 Connecting to Wi-Fi...");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\n✅ Wi-Fi Connected!");

    // Initialize LoRa Module
    Serial.println("🔄 Checking SPI connection...");
    LoRa.setPins(SS, RST, DIO0); 

    if (!LoRa.begin(865E6)) {  
        Serial.println("❌ LoRa Initialization FAILED!");
        while (1);
    }

    Serial.println("✅ LoRa Initialized Successfully!");
    LoRa.setSpreadingFactor(7);
    LoRa.setSignalBandwidth(125E3);
    LoRa.setCodingRate4(5);
    LoRa.enableCrc();
}

void loop() {
    Serial.println("🔍 Checking for packets...");
    int packetSize = LoRa.parsePacket();
    
    if (packetSize) {
        Serial.println("📡 Packet received!");
        String receivedData = "";

        while (LoRa.available()) {
            receivedData += (char)LoRa.read();
        }

        Serial.print("📩 Received Moisture: ");
        Serial.println(receivedData);

        // Generate random values for N, P, K, and pH
        int n = random(10, 131);  
        int p = random(10, 131);  
        int k = random(10, 131);  
        float pH = random(40, 101) / 10.0;  

        Serial.print("🌿 N: "); Serial.print(n);
        Serial.print(" | P: "); Serial.print(p);
        Serial.print(" | K: "); Serial.print(k);
        Serial.print(" | pH: "); Serial.println(pH);

        sendToGoogleSheets(n, p, k, pH);
    }

    // 🔹 Check sheet for control command
    readFromGoogleSheets(2, 9);
    delay(100);
}

void sendToGoogleSheets(int n, int p, int k, float pH) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        int temp = tdm + 1;

        String values[4] = {String(n), String(p), String(k), String(pH, 1)};
        
        for (int col = 4; col <= 7; col++) {
            String requestURL = String(serverName) + "?action=update&row=" + String(temp) + "&column=" + String(col) + "&value=" + values[col - 4];

            http.begin(requestURL);
            int httpResponseCode = http.GET();
            
            if (httpResponseCode > 0) {
                Serial.print("✅ Column ");
                Serial.print(col);
                Serial.print(" updated. Response: ");
                Serial.println(httpResponseCode);
            } else {
                Serial.print("❌ Failed to update column ");
                Serial.print(col);
                Serial.print(". HTTP Error: ");
                Serial.println(httpResponseCode);
            }
            http.end();
            delay(300);
        }

        tdm = (tdm == 4) ? 1 : tdm + 1;
    } else {
        Serial.println("⚠ Wi-Fi Disconnected!");
    }
}

void readFromGoogleSheets(int row, int column) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        String requestURL = String(serverName) + "?action=read&row=" + String(row) + "&column=" + String(column);

        http.begin(requestURL);
        http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

        int httpResponseCode = http.GET();
        
        if (httpResponseCode > 0) {
            sheetValue = http.getString();
            sheetValue.trim();  // Clean any junk off ends
            Serial.print("📖 Read from Google Sheets: ");
            Serial.println(sheetValue);

            if (sheetValue == "ON" || sheetValue == "OFF") {
                unsigned long startTime = millis();
                Serial.println("📡 Starting repeated LoRa transmission for 5 seconds...");
                while (millis() - startTime < 5000) {
                    LoRa.beginPacket();
                    LoRa.print(sheetValue);
                    LoRa.endPacket();
                    Serial.print("✅ LoRa Sent: ");
                    Serial.println(sheetValue);
                    delay(500);  // Space 'em out just a tad
                }
                Serial.println("🛑 LoRa transmission complete.");
            } else {
                Serial.println("⚠ Sheet said somethin’ weird, not ON/OFF.");
            }
        } else {
            Serial.print("❌ Failed to read data. HTTP Error: ");
            Serial.println(httpResponseCode);
        }

        http.end();
    } else {
        Serial.println("⚠ Wi-Fi Disconnected!");
    }
}
