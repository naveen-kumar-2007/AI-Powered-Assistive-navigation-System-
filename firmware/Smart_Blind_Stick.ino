/*
  Project: AI-Powered Assistive Navigation System for Visually Impaired People
  Controller: ESP8266 NodeMCU
  Functions:
    1. HC-SR04 obstacle detection
    2. Vibration motor alert through a transistor driver
    3. NEO-6M GPS location upload to ThingSpeak

  IMPORTANT:
  - Confirm all pin connections and supply voltages before powering the circuit.
  - HC-SR04 ECHO is 5V: use a voltage divider before ESP8266 D2.
  - Never drive a vibration motor directly from an ESP8266 GPIO.
*/

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <SoftwareSerial.h>
#include <TinyGPSPlus.h>

// ---------- USER SETTINGS ----------
const char* WIFI_SSID = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const char* THINGSPEAK_WRITE_API_KEY = "YOUR_THINGSPEAK_WRITE_API_KEY";

// ---------- NODEMCU PIN CONNECTIONS ----------
const uint8_t TRIG_PIN = D1;
const uint8_t ECHO_PIN = D2;       // Must receive 3.3V maximum
const uint8_t MOTOR_DRIVER_PIN = D5;

// SoftwareSerial constructor: ESP8266 RX pin, ESP8266 TX pin
// GPS TX -> D6; GPS RX -> D7 (optional; not needed for basic GPS reading)
SoftwareSerial gpsSerial(D6, D7);
TinyGPSPlus gps;

// ---------- SETTINGS ----------
const float OBSTACLE_LIMIT_CM = 100.0;
const unsigned long UPLOAD_INTERVAL_MS = 20000; // ThingSpeak free tier: >=15 sec
const unsigned long GPS_MAX_AGE_MS = 10000;

unsigned long lastUploadTime = 0;
unsigned long lastDistanceReadTime = 0;
float distanceCm = -1.0;

float readDistanceCm() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(3);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // 30 ms timeout is approximately 5 m maximum range
  unsigned long duration = pulseIn(ECHO_PIN, HIGH, 30000UL);
  if (duration == 0) {
    return -1.0; // No echo
  }
  return (duration * 0.0343f) / 2.0f;
}

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print(F("Connecting to Wi-Fi"));
  unsigned long start = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - start < 20000UL) {
    delay(400);
    Serial.print('.');
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.print(F("Wi-Fi connected. IP: "));
    Serial.println(WiFi.localIP());
  } else {
    Serial.println();
    Serial.println(F("Wi-Fi connection failed. Check credentials."));
  }
}

void uploadToThingSpeak() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println(F("Wi-Fi disconnected; attempting reconnect."));
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    return;
  }

  if (!gps.location.isValid() || gps.location.age() > GPS_MAX_AGE_MS) {
    Serial.println(F("No fresh GPS fix; location not uploaded."));
    return;
  }

  WiFiClient client;
  HTTPClient http;

  String url = "http://api.thingspeak.com/update?api_key=";
  url += THINGSPEAK_WRITE_API_KEY;
  url += "&field1=" + String(distanceCm, 1);
  url += "&field2=" + String((distanceCm > 0 && distanceCm <= OBSTACLE_LIMIT_CM) ? 1 : 0);
  url += "&lat=" + String(gps.location.lat(), 6);
  url += "&long=" + String(gps.location.lng(), 6);

  if (http.begin(client, url)) {
    int httpCode = http.GET();
    String response = http.getString();

    Serial.print(F("ThingSpeak HTTP code: "));
    Serial.println(httpCode);
    Serial.print(F("ThingSpeak entry response: "));
    Serial.println(response);

    http.end();
  } else {
    Serial.println(F("Could not start ThingSpeak HTTP request."));
  }
}

void setup() {
  Serial.begin(115200);
  gpsSerial.begin(9600);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(MOTOR_DRIVER_PIN, OUTPUT);

  digitalWrite(TRIG_PIN, LOW);
  digitalWrite(MOTOR_DRIVER_PIN, LOW);

  Serial.println();
  Serial.println(F("Assistive Navigation System starting..."));

  connectWiFi();
}

void loop() {
  // Continuously parse incoming GPS NMEA data
  while (gpsSerial.available()) {
    gps.encode(gpsSerial.read());
  }

  // Read obstacle distance approximately every 100 ms
  if (millis() - lastDistanceReadTime >= 100UL) {
    lastDistanceReadTime = millis();
    distanceCm = readDistanceCm();

    bool obstacleDetected =
      (distanceCm > 0 && distanceCm <= OBSTACLE_LIMIT_CM);

    digitalWrite(MOTOR_DRIVER_PIN, obstacleDetected ? HIGH : LOW);

    Serial.print(F("Distance (cm): "));
    if (distanceCm < 0) {
      Serial.println(F("No echo"));
    } else {
      Serial.println(distanceCm);
    }

    if (obstacleDetected) {
      Serial.println(F("Obstacle detected - vibration ON"));
    }
  }

  // Upload at the selected interval
  if (millis() - lastUploadTime >= UPLOAD_INTERVAL_MS) {
    lastUploadTime = millis();

    if (gps.location.isValid()) {
      Serial.print(F("Latitude: "));
      Serial.println(gps.location.lat(), 6);
      Serial.print(F("Longitude: "));
      Serial.println(gps.location.lng(), 6);
    }

    uploadToThingSpeak();
  }

  yield();
}
