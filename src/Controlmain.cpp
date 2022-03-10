#include <Arduino.h>

#define MOTOR_PIN_1 6
#define MOTOR_PIN_2 7
#define MOTOR_PIN_3 8
#define MOTOR_PIN_4 9

bool connectedToApp = false;

int wait_time = 7; // time between cleanings
int operation_time = 5; // time for a sweep in one direction
int cooldown_time = 3; // minimum time for system to cooldown before next operation

float temperature_threshold; // minimum value for temperature sensor to accept its input
float humidity_threshold; // minimum value for humidity sensor to accept its input
float rain_threshold; // minimum value for rain sensor to accept its input
float daylight_threshold; // minimum value for daylight sensor to accept its input

void Clean() {
  digitalWrite(MOTOR_PIN_2, LOW);
  digitalWrite(MOTOR_PIN_4, LOW);
  sleep(2);
  digitalWrite(MOTOR_PIN_1, HIGH);
  digitalWrite(MOTOR_PIN_3, HIGH);
  sleep(operation_time); // Does this stop the operations?
  digitalWrite(MOTOR_PIN_1, LOW);
  digitalWrite(MOTOR_PIN_3, LOW);
  sleep(2);
  digitalWrite(MOTOR_PIN_2, HIGH);
  digitalWrite(MOTOR_PIN_4, HIGH);
  sleep(operation_time); // Does this stop the operations?
  digitalWrite(MOTOR_PIN_2, LOW);
  digitalWrite(MOTOR_PIN_4, LOW);
  sleep(cooldown_time);
}

void ReadSensors() {
  // Has to run while system is sleeping

  // Listens to sensors and calls Clean() if necessary

  /* if ((temperatureInput >= temperature_threshold) &&
        (humidityInput >= humidity_threshold) &&
        (rainInput >= rain_threshold) &&
        (daylightInput >= daylight_threshold)) {
      Clean();
    }
  */
}

void UpdateSettings() {
  // Check if any changes to variables came from app
}

void Connect() {
  // Connects to App
}

void Disconnect() {
  // Disconnect from App
}

void CheckAppConnection() {
  // Has to run while system is sleeping

  // Listen if app is on
  // If on, connect
  // Else, disconnect
  // sleep()
}

void setup() {
  // Set pinModes
  pinMode(MOTOR_PIN_1, OUTPUT);
  pinMode(MOTOR_PIN_2, OUTPUT); 
  pinMode(MOTOR_PIN_3, OUTPUT);
  pinMode(MOTOR_PIN_4, OUTPUT); 

  // CheckAppConnection()
}

void loop() {
  if (connectedToApp)
     UpdateSettings();
  Clean();
  sleep(wait_time - cooldown_time);
}