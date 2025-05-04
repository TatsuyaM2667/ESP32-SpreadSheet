#include <Arduino.h>
#include <WiFi.h>
#include "time.h"
#include <ESP_Google_Sheet_Client.h>
#include <Wire.h>

// For SD/SD_MMC mounting helper
#include <GS_SDHelper.h>

#define WIFI_SSID ""
#define WIFI_PASSWORD ""

// Google Project ID
#define PROJECT_ID ""

// Service Account's client email
#define CLIENT_EMAIL ""

// Service Account's private key
const char PRIVATE_KEY[] PROGMEM = "-----BEGIN PRIVATE KEY-----\\n-----END PRIVATE KEY-----\n";


// The ID of the spreadsheet where you'll publish the data
const char spreadsheetId[] = "";

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 30000;

// Token Callback function
void tokenStatusCallback(TokenInfo info);

// NTP server to request epoch time
const char* ntpServer = "pool.ntp.org";

// Variable to save current epoch time
unsigned long epochTime;

// Buffer to hold I2C data
String i2cTextData = "";

// Function that gets current epoch time
unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    //Serial.println("Failed to obtain time");
    return(0);
  }
  time(&now);
  return now;
}

// Function to handle received I2C data
void receiveEvent(int numBytes) {
  while (Wire.available()) {
    char c = Wire.read();
    i2cTextData += c;
  }
}

void setup() {

    Serial.begin(115200);
    Serial.println();
    Serial.println();
    pinMode(15,OUTPUT);
    // Initialize I2C
    Wire.begin(8); // Replace 8 with your I2C address
    Wire.onReceive(receiveEvent);

    // Configure time
    configTime(0, 0, ntpServer);

    GSheet.printf("ESP Google Sheet Client v%s\n\n", ESP_GOOGLE_SHEET_CLIENT_VERSION);

    // Connect to Wi-Fi
    WiFi.setAutoReconnect(true);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(1000);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    // Set the callback for Google API access token generation status (for debug only)
    GSheet.setTokenCallback(tokenStatusCallback);

    // Set the seconds to refresh the auth token before expire (60 to 3540, default is 300 seconds)
    GSheet.setPrerefreshSeconds(10 * 60);

    // Begin the access token generation for Google API authentication
    GSheet.begin(CLIENT_EMAIL, PROJECT_ID, PRIVATE_KEY);
}

void loop() {
    // Call ready() repeatedly in loop for authentication checking and processing
    bool ready = GSheet.ready();

    if (ready && millis() - lastTime > timerDelay) {
        lastTime = millis();

        FirebaseJson response;

        Serial.println("\nAppend spreadsheet values...");
        Serial.println("----------------------------");

        FirebaseJson valueRange;

        // Get timestamp
        epochTime = getTime();

        // Add timestamp and I2C text data to valueRange
        valueRange.add("majorDimension", "COLUMNS");
        valueRange.set("values/[0]/[0]", epochTime);
        valueRange.set("values/[1]/[0]", i2cTextData);

        // For Google Sheet API ref doc, go to https://developers.google.com/sheets/api/reference/rest/v4/spreadsheets.values/append
        // Append values to the spreadsheet
        bool success = GSheet.values.append(&response /* returned response */, spreadsheetId /* spreadsheet Id to append */, "Sheet1!A1" /* range to append */, &valueRange /* data range to append */);
        if (success) {
            response.toString(Serial, true);
            valueRange.clear();
            i2cTextData = ""; // Clear the I2C data buffer after writing to the spreadsheet
        } else {
            Serial.println(GSheet.errorReason());
        }
        Serial.println();
        Serial.println(ESP.getFreeHeap());
    }
}

void tokenStatusCallback(TokenInfo info) {
    if (info.status == token_status_error) {
        GSheet.printf("Token info: type = %s, status = %s\n", GSheet.getTokenType(info).c_str(), GSheet.getTokenStatus(info).c_str());
        GSheet.printf("Token error: %s\n", GSheet.getTokenError(info).c_str());
    } else {
        GSheet.printf("Token info: type = %s, status = %s\n", GSheet.getTokenType(info).c_str(), GSheet.getTokenStatus(info).c_str());
        digitalWrite(15,HIGH);
        delay(200);
        digitalWrite(15,LOW);
    }
}
