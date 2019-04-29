/*
 Name:		BatteryDrainTest.ino
 Created:	4/8/2019 9:11:50 AM
 Author:	Jason Reichard

 This is a program to run a battery drain test by recording the battery level, connecting to WiFi, posting results to a Google sheet, 
 and then going to sleep for a specified time. It repeats until the battery is fully discharged.
*/

#include "BraceletHardware.h"

// Internet Stuff
WiFiClientSecure client;
const char* host = "script.google.com";
const char* scriptID = "YourGoogleScriptIDHere";
String url;
const int httpPort = 443;

// WiFi config
char *ssid = "YourSSIDHere";
char *pass = "YourPasswordHere";

// boolean to enable debug text
const boolean debug = 1;

// Battery & wifi
Battery myBattery;
braceletWifi myWifi(ssid, pass, 1);

// sleep duration in ms
int sleepTime = 30000;

double batV;
int runtime;

void setup() {
	hwInit();
}

void loop() {
	myBattery.charge();
	batV = myBattery.batVoltage; 
	runtime = millis();
	myWifi.connect();
	postData();

	if (myBattery.pluggedIn) {
		if (debug) {
			Serial.println("plugged in");
		}
		delay(5000);
	} else {
		if (debug) {
			Serial.println("going to sleep");
		}
		lightSleep(sleepTime);
		if (debug) {
			Serial.println("woke up");
		}
	}
}

// posts batV & millis to google sheets
void postData() {
	if (debug) {
		Serial.println("Posting Data");
		Serial.print("WiFi Status: ");
		Serial.println(WiFi.status());
		Serial.print("connecting to ");
		Serial.println(host);
	}

reconnect:
	if (!client.connect(host, httpPort)) {
		delay(500);
		if (debug) {
			Serial.println("Connection failed");
		}
		goto reconnect;
	} else {
		delay(1);
		if (debug) {
			Serial.println("connected");
		}
	}

	url = "/macros/s/" + String(scriptID) + "/exec?func=addData&batV=" + String(batV, 3) + "&millis=" + String(runtime);

	if (debug) {
		Serial.print("Requesting URL: ");
		Serial.println(url);
	}

	client.print(
		String("GET ")
		+ url
		+ " HTTP/1.1\r\n"
		+ "Host: "
		+ host
		+ "\r\n"
		+ "Connection: close\r\n\r\n"
	);
	delay(500);

	if (debug) {
		Serial.println("closing connection");
		Serial.println("");
	}
	client.stop();
}
