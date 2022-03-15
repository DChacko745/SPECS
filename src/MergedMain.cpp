#include <Arduino.h>
#include <WiFi.h>

#define MOTOR_PIN_1 25
#define MOTOR_PIN_2 26
#define MOTOR_STOP 9

// Replace with your network credentials
const char* ssid = "iPhone";
const char* password = "esp32Connection";

WiFiServer server(80); // Set web server port number to 80

bool connectedToApp = false;

int wait_time = 7; // time between cleanings
int operation_time = 5; // time for a sweep in one direction
int cooldown_time = 5; // minimum time for system to cooldown before next operation

float temperature_threshold; // minimum value for temperature sensor to accept its input
float humidity_threshold; // minimum value for humidity sensor to accept its input
float rain_threshold; // minimum value for rain sensor to accept its input
float daylight_threshold; // minimum value for daylight sensor to accept its input

String header; // Variable to store the HTTP request
String LEDState = "off"; // Auxiliar variables to store the current output state

unsigned long currentTime = millis(); // Current time
unsigned long previousTime = 0; // Previous time

const long timeoutTime = 2000; // Define timeout time in milliseconds (example: 2000ms = 2s)

void Clean() {
  //sleep(cooldown_time);
  digitalWrite(MOTOR_PIN_1, HIGH);
  sleep(operation_time); // Does this stop the operations?
  digitalWrite(MOTOR_PIN_1, LOW);
  sleep(2);
  digitalWrite(MOTOR_PIN_2, HIGH);
  sleep(operation_time); // Does this stop the operations?
  digitalWrite(MOTOR_PIN_2, LOW);
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
  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

//handle incoming clients
void handleClient(WiFiClient client) {
  currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // turns the GPIOs on and off
            if (header.indexOf("GET /led/on") >= 0) {
              Serial.println("Run Cleaning");
              Clean();
              LEDState = "on";
            } else if (header.indexOf("GET /led/off") >= 0) {
              LEDState = "off";
            }             
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>SPECS Wiper System</h1>");
            
            // Display current state, and ON/OFF buttons for GPIO led  
            client.println("<p>Run a Cleaning</p>");
            // If the LEDState is off, it displays the ON button       
            if (LEDState=="off") {
              client.println("<p><a href=\"/led/on\"><button class=\"button\">Clean</button></a></p>");
            } else {
              client.println("<p><a href=\"/led/off\"><button class=\"button button2\">Cleaning</button></a></p>");
            } 
               
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }

}

void setup() {
  // Set pinModes
  pinMode(MOTOR_PIN_1, OUTPUT);
  pinMode(MOTOR_PIN_2, OUTPUT); 
  pinMode(MOTOR_STOP, INPUT_PULLUP);
  digitalWrite(MOTOR_PIN_1, LOW);
  digitalWrite(MOTOR_PIN_2, LOW);

  Serial.begin(115200);
  CheckAppConnection();
}

void loop() {
  //always check for clients trying to connect to our server 
  WiFiClient client = server.available();

  if (client) {                             // If a new client connects,
        handleClient(client);
        client.stop();      //temp: to prevent infinite loop 
    }
    header = "";
  /*if (digitalRead(MOTOR_STOP) == HIGH) {
    Serial.print("I have stopped! Yay");
  }*/
  //if (connectedToApp)
     //UpdateSettings();

  //Clean();

  //sleep(wait_time - cooldown_time);
}
