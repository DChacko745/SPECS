#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <time.h>

//ESP Pin Definitions:
#define MOTOR_PIN_1 25
#define MOTOR_PIN_2 26
#define MOTOR_STOP_1 32
#define MOTOR_STOP_2 33

#define DAYLIGHT_SENSOR_PIN 14
#define HT_SENSOR_PIN 27
#define DHTTYPE DHT11
DHT dht(HT_SENSOR_PIN, DHTTYPE);

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
String rainPath;
String powerGenPath;
String powerStoPath;
String isUserConPath;
String lastCleanPath;
String lastSensReadPath;
String nextCleanPath;
String isAutoCleanOnPath;
String isCleaningPath;
String isRefreshingPath;

// Current System Settings
bool isAutoCleanOn = false;

// DHT11 sensor
float temperature;
float humidity;
float daylight;
float rain;

float temperature_threshold; // minimum value for temperature sensor to accept its input (in Fahrenheit)
float humidity_threshold; // minimum value for humidity sensor to accept its input (20-80%)
float rain_threshold; // minimum value for rain sensor to accept its input
float daylight_threshold; // minimum value for daylight sensor to accept its input (0-4096)

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
    Firebase.RTDB.setFloat(&fbdo, path.c_str(), value);
    /*
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
    */
}

// Retrieve sensor readings
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

// Send new readings to database
void UpdateSensorData_FB() {
    sendDataPrevMillis = millis();
 
    // Get latest sensor readings
    ReadSensors();

    // Send readings to database:
    sendFloat(tempPath, temperature);
    sendFloat(humPath, humidity);
    sendFloat(dayPath, daylight);

    Firebase.RTDB.setBool(&fbdo, isRefreshingPath.c_str(), false);
}

// Controls wiper movement
void Clean() {
    digitalWrite(MOTOR_PIN_1, HIGH); // Runs motor clockwise
    while (digitalRead(MOTOR_STOP_1) == LOW); // Waits until wiper reaches end of panel
    digitalWrite(MOTOR_PIN_1, LOW); // Stop
    delay(2000); // Wait 2 seconds
    digitalWrite(MOTOR_PIN_2, HIGH); // Runs motor counterclockwise
    while (digitalRead(MOTOR_STOP_2) == LOW); // Waits until wiper reaches end of panel
    digitalWrite(MOTOR_PIN_2, LOW); // Stop
    delay(cooldown_time);

    Firebase.RTDB.setBool(&fbdo, isCleaningPath.c_str(), false);
}

// Checks if any system settings changed
void CheckSystemSettings_FB() {
    bool tempIsAutoCleanOn;
    bool IsCleaning;
    bool IsRefreshing;
    
    Firebase.RTDB.getBool(&fbdo, isAutoCleanOnPath, &tempIsAutoCleanOn);
    Serial.println(tempIsAutoCleanOn);
    Firebase.RTDB.getBool(&fbdo, isCleaningPath, &IsCleaning);
    Serial.println(IsCleaning);
    Firebase.RTDB.getBool(&fbdo, isRefreshingPath, &IsRefreshing);
    Serial.println(IsRefreshing);

    if (tempIsAutoCleanOn != isAutoCleanOn)
        isAutoCleanOn = tempIsAutoCleanOn;
    if (IsCleaning)
        Clean();
    if (IsRefreshing)
        UpdateSensorData_FB();
}

void setup(){
    // Set pinModes
    pinMode(MOTOR_PIN_1, OUTPUT);
    pinMode(MOTOR_PIN_2, OUTPUT); 
    pinMode(MOTOR_STOP_1, INPUT_PULLUP);
    pinMode(MOTOR_STOP_2, INPUT_PULLUP);

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
    databasePath = "/Users/test/deviceMacAddress";
    //databasePath = "/Users/" + uid + "/deviceMacAddress";

    // Update database path for sensor readings
    tempPath = databasePath + "/sensorData/temperature";
    humPath = databasePath + "/sensorData/humidity";
    dayPath = databasePath + "/sensorData/daylight";
    rainPath = databasePath + "/sensorData/rain";
    powerGenPath = databasePath + "/sensorData/PowerGenerated";
    powerStoPath = databasePath + "/sensorData/PowerStored";
    isUserConPath = databasePath + "/systemData/isUserConnected";
    lastCleanPath = databasePath + "/systemData/lastClean";
    lastSensReadPath = databasePath + "/systemData/lastSensorRead";
    nextCleanPath = databasePath + "/systemData/nextClean";
    isAutoCleanOnPath = databasePath + "/systemSettings/isAutoCleanOn";
    isCleaningPath = databasePath + "/systemSettings/isCleaning";
    isRefreshingPath = databasePath + "/systemSettings/isRefreshing";

    dht.begin();
}

void loop(){
  if (Firebase.ready() && (millis() - sendDataPrevMillis > timerDelay || sendDataPrevMillis == 0)) {
    //UpdateSensorData_FB();
    CheckSystemSettings_FB();
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