#include <Streaming.h>
#include <ESP8266WiFi.h>
#include <Ticker.h>
#include "BraceletHardware.h"
// Information for connecting to the network
char* ssid = "E45";
char* password = "ihavefallenandicantgetup";
// IP address for the server
const char* host = "192.168.42.1";
// Creating necessary variables for the program
bool connection = false;
Ticker vibrate;
double batV;
long runtime;
String alert;
int count = 0;
Battery myBattery;
braceletWifi myWifi(ssid, password, 1);
// Method for vibrating the motor when an alert is received
void vibrateMotor() {
  int state = digitalRead(PAGER_MOTOR);  
  digitalWrite(PAGER_MOTOR, !state); 
  count++;
  if(count==10) {
    digitalWrite(PAGER_MOTOR, LOW);
    count = 0;
    vibrate.detach();
  }
}

void setup()
{
  // Intitialize all the pins and enable serial monitoring
  hwInit();
}


void loop()
{
  // Connect to the network and update battery information
  myWifi.connect();
  Serial.println("Connected to the Wifi");
  myBattery.charge();
  // Start Wifi client that will communicate with the server
  WiFiClient client;
  Serial.printf("\n[Connecting to %s ... ", host);
  // Connect to the server using the IP address of the server
  if ((client.connect(host, 43000)))
  {
    Serial.println("connected]");
    connection = true;
    Serial.println("[Sending a request]");
    // Identify the device as a bracelet
    client.print("esp");
    // Receives and prints response from the server
    Serial.println("[Response:]");
    String response = client.readString();
    Serial.println(response);
    // The ESP creates a str that contains information on whether the button has been pressed
    char str[10];
    strcpy(str, "CHECK IN:");
    if(buttonPressed){
      strcat(str, "1");
      // The LED will blink red when an alert is being sent
      digitalWrite(RLED, HIGH);
      delay(500);
      digitalWrite(RLED, LOW);
      buttonPressed = false;
    }
    // If no alert is given a 0 is sent to the server
    else {
      strcat(str, "0");
    }
    // The adjusted string is then sent to the server
    client.print(str);
    Serial.println(str);
    // The MacAddress is sent to identify the bracelet
    client.print(WiFi.macAddress());
    // The ESP receives a message that tells if the server sent out an alert
    alert = client.readString();
    Serial.println(alert);
    // If an alert is received the bracelet will vibrate
    if(alert == "ALERT") {
      vibrate.attach(0.5, vibrateMotor); 
    }
    // The bracelet then disconnects from the server
    client.stop();
    Serial.println("\n[Disconnected]");
  }
  // If the bracelet cannot connect to the server the client is closes
  else
  {
    Serial.println("connection failed!]");
    client.stop();
  }
  // If the bracelet is plugged in then there is no need for sleep
  if(myBattery.pluggedIn){
    if(digitalRead(BUTTON)){
      buttonPressed=true;
    }
  }
//  // Once everything has completed the bracelet will sleep 
  else {
    lightSleep(30000);
  }
 

}
