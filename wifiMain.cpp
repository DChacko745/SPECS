#include <WiFi.h>

// Replace with your network credentials
const char* ssid = "iPhone";
const char* password = "connection";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

//handle incoming clients
void handleClient(WiFiClient client) {
  Serial.println("New Client.");          // print a message out in the serial port
  String currentLine = "";                // make a String to hold incoming data from the client
  while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
          char c = client.read();             // read a byte, then
          Serial.write(c);                    // print it out the serial monitor
          header += c;
          if(c == '\n') {
            if(currentLine.length() == 0) {
              client.println("HTTP/1.1 200 OK");
              client.println("Content-type:text/html");
              client.println("Connection: close");
              client.println();

              client.println("<!DOCTYPE html><html>");
              client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
              client.println("<link rel=\"icon\" href=\"data:,\">");
              
              // CSS to style the on/off buttons 
              // Feel free to change the background-color and font-size attributes to fit your preferences
              client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
              //client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
              client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}</style></head>");
              //client.println(".button2 {background-color: #555555;}</style></head>");
              
              // Web Page Heading
              client.println("<body><h1>SPECS Web Server</h1>");
              client.println("<body><h1>Welcome</h1>");
              client.println("</body></html>");
              client.println();               
              
              break;
            }
          
            // if you got a newline, then clear currentLine
            else { currentLine = ""; }
            // if you got anything else but a carriage return character,
            // add it to the end of the currentLine
          }   
          else if (c != '\r') { currentLine += c; }
      }
          //client.println("Sending back");
  }

}

void setup() {
  Serial.begin(115200);
  
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

void loop(){
    //always check for clients trying to connect to our server 
    WiFiClient client = server.available();   

    if (client) {                             // If a new client connects,
        handleClient(client);
        client.stop();      //temp: to prevent infinite loop 
    }
    // Clear the header variable
    header = "";
    // Close the connection
    //client.stop();
    //Serial.println("Client disconnected.");
    //Serial.println("");
}

