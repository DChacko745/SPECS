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

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -240;
const int   daylightOffset_sec = 3600;

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

String cleaningIntervalPath;
String isUserConPath;
String lastCleanPath;
String lastSensReadPath;
String nextCleanPath;

String isAutoCleanOnPath;
String isCleaningPath;
String isRefreshingPath;
String inFahrenheitPath;

// Current System Settings
bool isAutoCleanOn = false;
bool inFahrenheit = true;
int cleaningInterval = 300000;

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

// Timer variables
unsigned long sendDataPrevMillis = 0;
unsigned long cleanPrevMillis = 0;
unsigned long sensorDatatimerDelay = 30000;

struct tm timeinfo;

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
  temperature = dht.readTemperature(inFahrenheit);
  daylight = analogRead(DAYLIGHT_SENSOR_PIN);
  // NEED TO ADD RAIN AND POWER SENSORS

  Serial.println("Humidity:");
  Serial.println(humidity, 5);
  Serial.println("Temperature:");
  Serial.println(temperature, 5);
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

    getLocalTime(&timeinfo);
    Firebase.RTDB.set(&fbdo, lastCleanPath.c_str(), &timeinfo);
    Firebase.RTDB.setBool(&fbdo, isRefreshingPath.c_str(), false);
}

// Controls wiper movement
void Clean() {
    cleanPrevMillis = millis();

    digitalWrite(MOTOR_PIN_1, HIGH); // Runs motor clockwise
    while (digitalRead(MOTOR_STOP_1) == LOW); // Waits until wiper reaches end of panel
    digitalWrite(MOTOR_PIN_1, LOW); // Stop
    delay(2000); // Wait 2 seconds
    digitalWrite(MOTOR_PIN_2, HIGH); // Runs motor counterclockwise
    while (digitalRead(MOTOR_STOP_2) == LOW); // Waits until wiper reaches end of panel
    digitalWrite(MOTOR_PIN_2, LOW); // Stop
    delay(cooldown_time);

    getLocalTime(&timeinfo);
    Firebase.RTDB.set(&fbdo, lastCleanPath.c_str(), &timeinfo);
    Firebase.RTDB.setBool(&fbdo, isCleaningPath.c_str(), false);
}

// Checks if any system settings changed
void CheckSystemSettings_FB() {
  bool temp_IsAutoCleanOn;
  bool temp_InFahrenheit;
  bool IsCleaning;
  bool IsRefreshing;
  int temp_cleaningInterval;
    
  Firebase.RTDB.getBool(&fbdo, isAutoCleanOnPath, &temp_IsAutoCleanOn);
  Serial.println(temp_IsAutoCleanOn);
  Firebase.RTDB.getBool(&fbdo, isCleaningPath, &IsCleaning);
  Serial.println(IsCleaning);
  Firebase.RTDB.getBool(&fbdo, isRefreshingPath, &IsRefreshing);
  Serial.println(IsRefreshing);
  Firebase.RTDB.getBool(&fbdo, inFahrenheitPath, &temp_InFahrenheit);
  Serial.println(temp_InFahrenheit);
  Firebase.RTDB.getInt(&fbdo, cleaningIntervalPath, &temp_cleaningInterval);
  Serial.println(temp_cleaningInterval);

  if (temp_IsAutoCleanOn != isAutoCleanOn)
    isAutoCleanOn = temp_IsAutoCleanOn;
  if (IsCleaning)
    Clean();
  if (IsRefreshing)
    UpdateSensorData_FB();
  if (temp_InFahrenheit != inFahrenheit)
    inFahrenheit = temp_InFahrenheit;
  if ((temp_cleaningInterval * 1000) != cleaningInterval) {
    cleaningInterval = temp_cleaningInterval;
    cleanPrevMillis = 1; // OR NOT?
  }
}

// Prints the current time
void printLocalTime()
{
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

void setup(){
    // Set pinModes
    pinMode(MOTOR_PIN_1, OUTPUT);
    pinMode(MOTOR_PIN_2, OUTPUT); 
    pinMode(MOTOR_STOP_1, INPUT_PULLUP);
    pinMode(MOTOR_STOP_2, INPUT_PULLUP);

    Serial.begin(115200);

    initWiFi();

    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    printLocalTime();

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
    //databasePath = "/Users/" + uid + "/" + WiFi.macAddress();

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
    inFahrenheitPath = databasePath + "/systemSettings/tempInFahrenheit";

    dht.begin();

    Firebase.RTDB.setString(&fbdo, "/Users/test/Name", "SPECS Team");
    //Firebase.RTDB.setString(&fbdo, "/Users" + uid + "/Name", "SPECS Team");

    Firebase.RTDB.setString(&fbdo, databasePath + "/Name", "Home Wiper");
}

void loop(){
  if (Firebase.ready() && (millis() - sendDataPrevMillis > sensorDatatimerDelay || sendDataPrevMillis == 0))
    UpdateSensorData_FB();

  if (Firebase.ready() && (millis() - cleanPrevMillis > cleaningInterval || cleanPrevMillis == 0))
    Clean();

  CheckSystemSettings_FB();
}

/**
 * Analyze
 * CheckSystemSettings()
 * Listeners on all systemSettings booleans in Firebase Database
 * 
 */