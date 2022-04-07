#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <time.h>

#define HT_SENSOR_PIN 27
#define DHTTYPE DHT11
DHT dht(HT_SENSOR_PIN, DHTTYPE);

#define DAYLIGHT_SENSOR_PIN 14

// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "iPhone"
#define WIFI_PASSWORD "esp32Connection"

// Insert Firebase project API Key
#define API_KEY "AIzaSyBtqq67zxXc9Az7AqhEWNbtQd2zyr8PyvY"

// Insert Authorized Email and Corresponding Password
#define USER_EMAIL "dchacko1@pride.hofstra.edu"
#define USER_PASSWORD "test12"

// Insert RTDB URLefine the RTDB URL
#define DATABASE_URL "https://specs-ef726-default-rtdb.firebaseio.com/"

// Define Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Variable to save USER UID
String uid;

// Variables to save database paths
String databasePath;
String tempPath;
String humPath;
String dayPath;

// DHT11 sensor
float temperature;
float humidity;
float daylight;
float rain;

float temperature_threshold; // minimum value for temperature sensor to accept its input (in Fahrenheit)
float humidity_threshold; // minimum value for humidity sensor to accept its input (20-80%)
float rain_threshold; // minimum value for rain sensor to accept its input
float daylight_threshold; // minimum value for daylight sensor to accept its input (0-4096)

float operation_time = 5000; // time for a sweep in one direction
float cooldown_time = 5000; // minimum time for system to cooldown before next operation

// Timer variables (send new readings every three minutes)
unsigned long sendDataPrevMillis = 0;
unsigned long timerDelay = 10000;

// Initialize WiFi
void initWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  Serial.println();
}

// Write float values to the database
void sendFloat(String path, float value){
  if (Firebase.RTDB.setFloat(&fbdo, path.c_str(), value)){
    Serial.print("Writing value: ");
    Serial.print (value);
    Serial.print(" on the following path: ");
    Serial.println(path);
    Serial.println("PASSED");
    Serial.println("PATH: " + fbdo.dataPath());
    Serial.println("TYPE: " + fbdo.dataType());
  }
  else {
    Serial.println("FAILED");
    Serial.println("REASON: " + fbdo.errorReason());
  }
}

void ReadSensors() {
  humidity = dht.readHumidity();
  temperature = dht.readTemperature(true); // in Fahrenheit

  Serial.println("Humidity:");
  Serial.println(humidity, 5);
  Serial.println("Temperature:");
  Serial.println(temperature, 5);

  daylight = analogRead(DAYLIGHT_SENSOR_PIN);
  Serial.println("Daylight Value:");
  Serial.println(daylight);
}

void Clean() {

}

void setup(){
  Serial.begin(115200);

  // Initialize BME280 sensor
  initWiFi();

  // Assign the api key (required)
  config.api_key = API_KEY;

  // Assign the user sign in credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  // Assign the RTDB URL (required)
  config.database_url = DATABASE_URL;

  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);

  // Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  // Assign the maximum retry of token generation
  config.max_token_generation_retry = 5;

  // Initialize the library with the Firebase authen and config
  Firebase.begin(&config, &auth);

  // Getting the user UID might take a few seconds
  Serial.println("Getting User UID");
  while ((auth.token.uid) == "") {
    Serial.print('.');
    delay(1000);
  }
  // Print user UID
  uid = auth.token.uid.c_str();
  Serial.print("User UID: ");
  Serial.println(uid);

  // Update database path
  //databasePath = "/UsersData/" + uid;
  databasePath = "/Users/test/device1/sensorData";

  // Update database path for sensor readings
  tempPath = databasePath + "/temperature"; // --> UsersData/<user_uid>/temperature
  humPath = databasePath + "/humidity"; // --> UsersData/<user_uid>/humidity
  dayPath = databasePath + "/daylight"; // --> UsersData/<user_uid>/pressure

  dht.begin();
}

void loop(){
  // Send new readings to database
  if (Firebase.ready() && (millis() - sendDataPrevMillis > timerDelay || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();
 
    // Get latest sensor readings
    ReadSensors();

    // Send readings to database:
    sendFloat(tempPath, temperature);
    sendFloat(humPath, humidity);
    sendFloat(dayPath, daylight);
  }
}

/**
 * ConnectToFirebase()
 * Clean()
 * ReadSensors()
 * Analyze
 * CheckSystemSettings()
 * Listeners on all systemSettings booleans in Firebase Database
 * 
 * Clean()
 * Motor CW
 * while (circuit is not completed) {
 * }
 * Motor CCW
 * while (circuit is not completed) {
 * }
 * isCleaning = false for the firebase database
 */