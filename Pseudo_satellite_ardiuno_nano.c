#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <RTClib.h>
#include <Adafruit_VEML6070.h>
#include "DHT.h"

// ----------------- Pin Definitions -----------------
#define DHTPIN 2         // DHT11 data pin
#define DHTTYPE DHT11    // DHT 11
#define GAS_PIN 5        // MQ sensor digital pin (D0 output)
#define SD_CS 10         // SD card CS pin

// ----------------- Objects -----------------
DHT dht(DHTPIN, DHTTYPE);
Adafruit_VEML6070 uv = Adafruit_VEML6070();
RTC_DS3231 rtc;
File myFile;

// ----------------- Setup -----------------
void setup() {
  Serial.begin(9600);
  
  // Start DHT11
  dht.begin();

  // Start UV sensor
  uv.begin(VEML6070_1_T);  // 1x integration time

  // Gas sensor pin
  pinMode(GAS_PIN, INPUT);

  // Start RTC
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  // If RTC is not set, set it once (upload & comment this line later)
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // sets to PC compile time

  // Start SD card
  if (!SD.begin(SD_CS)) {
    Serial.println("SD card initialization failed!");
    while (1);
  }

  // Create CSV file with headers (only once)
  myFile = SD.open("datalog.csv", FILE_WRITE);
  if (myFile) {
    myFile.println("Date,Time,Temperature,Humidity,UV,GAS");
    myFile.close();
  }
}

// ----------------- Loop -----------------
void loop() {
  // Get Date & Time
  DateTime now = rtc.now();
 // Read DHT11
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // Read UV sensor
  uint16_t uvRaw = uv.readUV();
  
  // Read Gas Sensor (D0)
  int gasState = digitalRead(GAS_PIN);

  // Debug print
  Serial.print(now.timestamp(DateTime::TIMESTAMP_DATE));
  Serial.print(" ");
  Serial.print(now.timestamp(DateTime::TIMESTAMP_TIME));
  Serial.print(" -> Temp: "); Serial.print(t);
  Serial.print(" C  Hum: "); Serial.print(h);
  Serial.print(" %  UV: "); Serial.print(uvRaw);
  Serial.print("  GAS: "); Serial.println(gasState);

  // Write to SD card
  myFile = SD.open("datalog.csv", FILE_WRITE);
  if (myFile) {
    // Date
    myFile.print(now.day()); myFile.print("/");
    myFile.print(now.month()); myFile.print("/");
    myFile.print(now.year());
    myFile.print(",");

    // Time (HH:MM:SS)
    myFile.print(now.hour()); myFile.print(":");
    myFile.print(now.minute()); myFile.print(":");
    myFile.print(now.second());
    myFile.print(",");

    // Sensor data
    myFile.print(t); myFile.print(",");
    myFile.print(h); myFile.print(",");
    myFile.print(uvRaw); myFile.print(",");
    myFile.println(gasState);

    myFile.close();
  } else {
    Serial.println("Error opening datalog.csv");
  }
  delay(1000); // log every 1 second
}
