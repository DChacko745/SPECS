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
#define MOTOR_STOP_1 12
#define MOTOR_STOP_2 35

#define BATTERY_VOLTAGE_SENSOR_PIN 33
#define SYSTEM_CURRENT_SENSOR_PIN 32
#define RAIN_SENSOR_PIN 34
#define HT_SENSOR_PIN 27
#define DHTTYPE DHT11
DHT dht(HT_SENSOR_PIN, DHTTYPE);

// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -18000;
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
String rainPath;
String powerGenPath;
String systemPowDrawPath;
String batVoltPath;

String cleaningIntervalPath;
String lastCleanPath;
String lastSensReadPath;
String nextCleanPath;

String toggleAutoCleanPath;
String toggleIntervalCleaningPath;
String isCleaningPath;
String isRefreshingPath;
String isUpdatingIntervalPath;
String tempInFahrenheitPath;

// Current System Settings
bool toggleAutoClean = true;
bool toggleIntervalCleaning = true;
bool tempInFahrenheit = true;
int cleaningInterval = 300;

// DHT11 sensor
float temperature;
float humidity;
int rain;
float powerGenerated;
float systemPowerDraw;
float battery_voltage;
float system_current_draw;

const float VOLTAGE_CONVERSION_VALUE = 192.0;
const float CURRENT_CONVERSION_VALUE = 192.0;
const float PANEL_RESISTANCE = 1;
const float SYSTEM_RESISTANCE = 1;

float temperature_threshold = 0; // minimum value for temperature sensor to accept its input (in Fahrenheit)
float humidity_threshold = 0; // minimum value for humidity sensor to accept its input (20-80%)
float rain_threshold = 80; // minimum value for rain sensor to accept its input

float cooldown_time = 5000; // minimum time for system to cooldown before next operation

// Timer variables
unsigned long sendDataPrevMillis = 0;
unsigned long cleanPrevMillis = 0;
unsigned long sensorDataTimerDelay = 30000;
unsigned long cleanTimerDelay = 600000;

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

// Gets the current time
String getCurrentTime() {
  if(!getLocalTime(&timeinfo)){
    //Serial.println("Failed to obtain time");
    return "Could not get current time";
  }
  //Serial.println(&timeinfo, "%A, %B %d %Y %H:%M");
  char str[40];
  strftime(str, sizeof(str), "%A, %B %d, %Y %H:%M", &timeinfo);
  return String(str);
}

// Calculate next clean
String getNextCleanTime() {
  getCurrentTime();
  struct tm nextCleanTimeInfo = timeinfo;

  nextCleanTimeInfo.tm_sec += cleaningInterval;
  mktime(&nextCleanTimeInfo);

  char str[40];
  strftime(str, sizeof(str), "%A, %B %d, %Y %H:%M", &nextCleanTimeInfo);
  return String(str);
}

// Write float values to the database
void sendFloat(String path, float value){
    Firebase.RTDB.setFloat(&fbdo, path.c_str(), value);
}

// Retrieve sensor readings
void ReadSensors() {
  Firebase.RTDB.setBool(&fbdo, isRefreshingPath.c_str(), true);

  humidity = dht.readHumidity();
  temperature = dht.readTemperature(tempInFahrenheit);
  int rainTemp = -1*analogRead(RAIN_SENSOR_PIN);
  rain = map(rainTemp,-4096,0,0,100);

  battery_voltage = analogRead(BATTERY_VOLTAGE_SENSOR_PIN) / VOLTAGE_CONVERSION_VALUE;
  powerGenerated = pow(battery_voltage, 2) / PANEL_RESISTANCE;
  system_current_draw = analogRead(SYSTEM_CURRENT_SENSOR_PIN) / CURRENT_CONVERSION_VALUE;
  systemPowerDraw = pow(system_current_draw, 2) * SYSTEM_RESISTANCE;

  /*
  Serial.println("Humidity:");
  Serial.println(humidity, 5);
  Serial.println("Temperature:");
  Serial.println(temperature, 5);
  Serial.println("Rain:");
  Serial.println(rain);
  Serial.println("Battery Voltage:");
  Serial.println(battery_voltage);
  Serial.println("Power Generated:");
  Serial.println(powerGenerated);
  Serial.println("System Current Draw:");
  Serial.println(system_current_draw);
  Serial.println("System Power Draw:");
  Serial.println(systemPowerDraw);
  */
}

// Send new readings to database
void UpdateSensorData_FB() {
    // Get latest sensor readings
    ReadSensors();

    // Send readings to database:
    sendFloat(tempPath, temperature);
    sendFloat(humPath, humidity);
    sendFloat(batVoltPath, battery_voltage);
    sendFloat(rainPath, rain);
    sendFloat(powerGenPath, powerGenerated);
    sendFloat(systemPowDrawPath, systemPowerDraw);
    
    Firebase.RTDB.setString(&fbdo, lastSensReadPath.c_str(), getCurrentTime());
    Firebase.RTDB.setBool(&fbdo, isRefreshingPath.c_str(), false);

    sendDataPrevMillis = millis();
}

// Updates cleaning interval
void UpdateCleaningInterval() {
  Firebase.RTDB.getInt(&fbdo, cleaningIntervalPath, &cleaningInterval);
  cleanPrevMillis = millis(); // Resets time
  Firebase.RTDB.setString(&fbdo, nextCleanPath, getNextCleanTime());
  Firebase.RTDB.setBool(&fbdo, isUpdatingIntervalPath.c_str(), false);
}

// Controls wiper movement
void Clean() {
    Firebase.RTDB.setBool(&fbdo, isCleaningPath.c_str(), true);

    digitalWrite(MOTOR_PIN_1, HIGH); // Runs motor clockwise
    Serial.print("Motor moves one direction");
    while (digitalRead(MOTOR_STOP_1) == LOW); // Waits until wiper reaches end of panel
    Serial.print("Motor stop 1 has circuit completed");
    digitalWrite(MOTOR_PIN_1, LOW); // Stop
    Serial.print("Motor stops and waits for 2 seconds");
    delay(2000); // Wait 2 seconds
    digitalWrite(MOTOR_PIN_2, HIGH); // Runs motor counterclockwise
    Serial.print("Motor moves in other direction");
    while (digitalRead(MOTOR_STOP_2) == LOW); // Waits until wiper reaches end of panel
    Serial.print("Motor stop 2 has circuit completed");
    digitalWrite(MOTOR_PIN_2, LOW); // Stop
    Serial.print("Motor stops");
    delay(cooldown_time);

    Firebase.RTDB.setString(&fbdo, lastCleanPath.c_str(), getCurrentTime());
    Firebase.RTDB.setString(&fbdo, nextCleanPath.c_str(), getNextCleanTime());
    Firebase.RTDB.setBool(&fbdo, isCleaningPath.c_str(), false);

    cleanPrevMillis = millis();
}

// Analyze sensor data for auto clean
bool AnalyzeSensorData() {
  ReadSensors();

  if (rain >= rain_threshold && temperature >= temperature_threshold && humidity >= humidity_threshold)
    return true;

  return false;
}

// Checks if any system settings changed
void CheckSystemSettings_FB() {
  bool temp_toggleAutoClean;
  bool temp_toggleIntervalCleaning;
  bool temp_tempInFahrenheit;
  bool IsCleaning;
  bool IsRefreshing;
  bool IsUpdatingInterval;
  
  Firebase.RTDB.getBool(&fbdo, toggleAutoCleanPath, &temp_toggleAutoClean);
  Firebase.RTDB.getBool(&fbdo, toggleIntervalCleaningPath, &temp_toggleIntervalCleaning);
  Firebase.RTDB.getBool(&fbdo, isCleaningPath, &IsCleaning);
  Firebase.RTDB.getBool(&fbdo, isRefreshingPath, &IsRefreshing);
  Firebase.RTDB.getBool(&fbdo, isUpdatingIntervalPath, &IsUpdatingInterval);
  Firebase.RTDB.getBool(&fbdo, tempInFahrenheitPath, &temp_tempInFahrenheit);

  // Checks if interval cleaning is toggled on or off
  if (temp_toggleIntervalCleaning != toggleIntervalCleaning) {
    toggleIntervalCleaning = temp_toggleIntervalCleaning;

    if (toggleIntervalCleaning)
      UpdateCleaningInterval();
    else // Probably won't be seen in web app but will be updated in database
      Firebase.RTDB.setString(&fbdo, nextCleanPath, "Interval Cleaning is Off!");
  }
  // If temperature units is changed in web app, update accordingly
  if (temp_tempInFahrenheit != tempInFahrenheit)
    tempInFahrenheit = temp_tempInFahrenheit;

  // If Clean button is pressed on web app, run Clean()
  if (IsCleaning)
    Clean();
  // If Refresh button is pressed on web app, update sensor data
  if (IsRefreshing)
    UpdateSensorData_FB();
  // If interval cleaning is toggled on in web app and "OK" button for cleaning interval is pressed, update cleaning interval
  if (toggleIntervalCleaning && IsUpdatingInterval)
    UpdateCleaningInterval();
}

void setup(){
    // Set pinModes
    pinMode(MOTOR_PIN_1, OUTPUT);
    pinMode(MOTOR_PIN_2, OUTPUT); 
    pinMode(MOTOR_STOP_1, INPUT_PULLDOWN);
    pinMode(MOTOR_STOP_2, INPUT_PULLDOWN);
    pinMode(BATTERY_VOLTAGE_SENSOR_PIN, INPUT_PULLDOWN);
    pinMode(SYSTEM_CURRENT_SENSOR_PIN, INPUT_PULLDOWN);
    pinMode(HT_SENSOR_PIN, INPUT);
    pinMode(RAIN_SENSOR_PIN, INPUT_PULLUP);

    Serial.begin(115200);

    initWiFi();

    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    getCurrentTime();

    // Assign the api key (required)
    config.api_key = API_KEY;

    // Assign the user sign in credentials
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;

    // Assign the RTDB URL (required)
    config.database_url = DATABASE_URL;

    Firebase.reconnectWiFi(true);
    fbdo.setResponseSize(4096);

    // Assign the callback function for the long running token generation task
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
    rainPath = databasePath + "/sensorData/rain";
    powerGenPath = databasePath + "/sensorData/powerGenerated";
    systemPowDrawPath = databasePath + "/sensorData/systemPowerDraw";
    batVoltPath = databasePath + "/sensorData/batteryVoltage";

    cleaningIntervalPath = databasePath + "/systemData/cleaningInterval";
    lastCleanPath = databasePath + "/systemData/lastClean";
    lastSensReadPath = databasePath + "/systemData/lastSensorRead";
    nextCleanPath = databasePath + "/systemData/nextClean";

    toggleAutoCleanPath = databasePath + "/systemSettings/toggleAutoClean";
    toggleIntervalCleaningPath = databasePath + "/systemSettings/toggleIntervalCleaning";
    isCleaningPath = databasePath + "/systemSettings/isCleaning";
    isRefreshingPath = databasePath + "/systemSettings/isRefreshing";
    isUpdatingIntervalPath = databasePath + "/systemSettings/isUpdatingInterval";
    tempInFahrenheitPath = databasePath + "/systemSettings/tempInFahrenheit";

    Firebase.RTDB.setString(&fbdo, "/Users/test/Name", "SPECS Team");
    //Firebase.RTDB.setString(&fbdo, "/Users" + uid + "/Name", "SPECS Team");

    Firebase.RTDB.setString(&fbdo, databasePath + "/Name", "Home Wiper");

    Firebase.RTDB.getBool(&fbdo, toggleAutoCleanPath, &toggleAutoClean);
    Firebase.RTDB.getBool(&fbdo, toggleIntervalCleaningPath, &toggleIntervalCleaning);
    Firebase.RTDB.getBool(&fbdo, tempInFahrenheitPath, &tempInFahrenheit);
    Firebase.RTDB.getInt(&fbdo, cleaningIntervalPath, &cleaningInterval);

    dht.begin();
}

void loop(){
  // Updates sensor data at set interval (currently at 30 seconds)
  if (Firebase.ready() && (millis() - sendDataPrevMillis > sensorDataTimerDelay || sendDataPrevMillis == 0)) {
    UpdateSensorData_FB();
  }

  // Cleans panel at set time interval
  if (toggleIntervalCleaning && Firebase.ready() && (millis() - cleanPrevMillis > cleaningInterval*1000)) {
    Clean();
  }
  
  // Cleans panel due to environmental factors (cooldown of 10 minutes)
  if (toggleAutoClean && Firebase.ready() && AnalyzeSensorData() && (millis() - cleanPrevMillis > cleanTimerDelay))
    Clean();

  CheckSystemSettings_FB();
}