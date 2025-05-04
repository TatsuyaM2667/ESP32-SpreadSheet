/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-datalogging-google-sheets/

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
  Adapted from the examples of the Library Google Sheet Client Library for Arduino devices: https://github.com/mobizt/ESP-Google-Sheet-Client

google spreadsheetの管理権をサービスアカウントに付与し、データを代入。
*/

#include <Arduino.h>
#include <WiFi.h>
#include "time.h"
#include <ESP_Google_Sheet_Client.h>
#include <Wire.h>

// For SD/SD_MMC mounting helper
#include <GS_SDHelper.h>

#define WIFI_SSID "Buildth"
#define WIFI_PASSWORD "Yasu098765567890"

// Google Project ID
#define PROJECT_ID "cansat-project"

// Service Account's client email
#define CLIENT_EMAIL "cansat-project-tatsuya-m@cansat-project.iam.gserviceaccount.com"

// Service Account's private key
const char PRIVATE_KEY[] PROGMEM = "-----BEGIN PRIVATE KEY-----\nMIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQD8KXJhGipTn7fP\n1GI27nAw9R1T2KLXWWtJ7Nk/FNEfjwwExQ25mVfbaU5lPOurM9/T65B6VlZ7+A3E\nNvEO0SRgk+jp0v3ntZNbizjo9YeZIw7o5N4LW3b+6nww9FOU63UotGVKNcuqoQfH\nWkAPLMbXa4yJiUx4WVPtKrJkL0GtVVy6RSvo0DThQ/DsbTPkPybKIV2ENo9nKadD\nbfwKNkwUQQWEIdrlLvS/PGJDiC+zeuyw2NCLLPYj1/VInYXTmeVgyizv3BMyvySC\n6asyBw1F9oEWXN4r7SZuTnvxxui5bdXppva9lLnCBaKwOzFaL6320EYfXOrRFK1j\n0FqX9dvZAgMBAAECggEAX3mZ8yYjDJKW2DwEC+Rv2jmUpyCJV5xlwRHKl9Xb8GUL\nGes6bSGgYFUY97//Q01a3DR++bMi/8FGBRsAXqnVajPWpmHa/RaE+s5V9+6vLs5V\nYNz3Lbmt7or+BtMbqnEUZRsYluaTfMGH3nVPd+9uQjZYeeLgb1jzLANUlBZIecXt\n9YxFXDrk6U5OHKZEzdbGMccJLCI74ZNNDok/ac/EwQQ8lfTJcPXTV2Z+vUnmMPcx\nJ0J6rFEwb7rVnPDS3rBxK8+CE1F1wt/3DMqN62ao0KGDdWDFjtZuTxHwzCzxdbjD\n8Ki48coWyctksc6BQisPgbB325Yu7BNXJU6LyIgpWQKBgQD+QLA3YkJQ78JcKTRS\nG6wcqpFJVuH62w+i+vt+uL4JEiFNCW+clzK5/YjfYdMhc5axiTH/z2TTdqaDH/9N\n0jruE3hm0t4yPwiY1r4vw6kA9ZFNdEPK13nW4TxJpSJSjt317KqGBT1wEDUHdZ4n\n7IEknvTHJVistk1Nyv3y8yEF/wKBgQD95RSAj2H9u7O5/wjqKNrNdH+P7MDtO2mS\nsnrOK4h+lvwHW/Vz9FHQcKzTFhwa07ys5x0g7YKNLWBFKMdSm2L4aePMZd/DZr9r\nNU3kOqyo2OHzdCpWCsStcE4t8cvV1ZyMI7qfnWIILzPuWVhQ9ThUmKBS4dqzu8zO\n3ef0pGYOJwKBgHKq0A6aZRhFmZ2hA7TIQC5Jiu7NnWtaosm5cfVr8mD66UPqLokr\n87fILZO5ddQPYnY9Tux1J57rUxuuMaFyYBuBcnrFPWrh3Mm5h9rY3n7NQbPx7KUt\n2whbWqpILJuZTWedTibVdCj3VkM/+407796JRhIOuNhj+6dLFo8csdcXAoGBAPf0\ntaUoYsbufYP0crxc0fM9ByeETJZjI6PE8vJOyar3wYNeZ8SqowwyPlaYjTIV/3nM\n7EGwM4O1AQQJ4z/lyS5aG8MO6w1mVyNYl42BUAZGacTdeks1kQbhWzi7oBph19V1\nP1mK1qavksQANaQ7GJxlwo2vTWuiyU5uoY3m0FSZAoGBALk27nyrhVPcpzMI4sDK\nL16Q7uKdfF3Tia3QnY3cQ5U3LRr+idGDG8Umqc6StaLwH0V5FbWiIFENe1IQUTYH\nnaMU/eZT751TUk1SYUSl5111rh/iwzvV8/HMfRwAUabzXsnBSODXKnkaTFaO2E9G\ncBR9hexye5OSbmshuYLNTswg\n-----END PRIVATE KEY-----\n";


// The ID of the spreadsheet where you'll publish the data
const char spreadsheetId[] = "1NZj-mIqYaTrVVLJTPCAyMgL6j7_cbMbtcYt9VnHwm7k";

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
