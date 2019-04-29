#pragma once

/*
	Name:       BraceletHardware.h
	Created:	4/8/2019
	Author:     Jason Reichard
*/

#include <ESP8266WiFi.h>
#include <Streaming.h>

extern "C" {
	#include "user_interface.h"
}

// Button state
bool buttonPressed = false;

// Pin definitions
const int RLED = 5;
const int GLED = 4;
const int PAGER_MOTOR = 13;
const int BUTTON = 16;
const int BAT_CHECK = 14;
const int CHARGE_EN = 12;
const int VBAT_SCALED = A0;

// hardware setup stuff
void hwInit(boolean serialDisable = 0) {
	if (serialDisable) {
		Serial.end();
	} else {
		Serial.begin(115200);
	}

	// configure GPIO pins
	pinMode(RLED, OUTPUT);
	pinMode(GLED, OUTPUT);
	pinMode(PAGER_MOTOR, OUTPUT);
	pinMode(BUTTON, INPUT);
	pinMode(BAT_CHECK, OUTPUT);
	pinMode(VBAT_SCALED, INPUT);
	pinMode(CHARGE_EN, OUTPUT);

	// sets up our output pins to be in the correct state upon startup because the ESP is dumb and likes to set some pins high upon boot for reasons beyond me
	digitalWrite(CHARGE_EN, 0);
	digitalWrite(BAT_CHECK, 0);
	digitalWrite(RLED, 0);
	digitalWrite(GLED, 0);
	digitalWrite(PAGER_MOTOR, 0);
}

// map function, but with doubles
double mapd(double x, double in_min, double in_max, double out_min, double out_max) {
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// returns the battery voltage when passed an adc reading (0 - 1023)
double adcToVolts(double adcReading) {
	return (adcReading / 1024.0)*4.27;
}

// colors the LED from red to green (0 = red, 255 = green, 128 = yellow/orange)
// values greater than 255 default to 255
// values less than 0 default to 0
void ledGradient(int n) {
	int level = n;
	if (n > 255) {
		level = 255;
	}
	if (n < 0) {
		level = 0;
	}
	analogWrite(RLED, 255 - level);
	analogWrite(GLED, level);
}

// turns both LEDs off
void LEDOff() {
	digitalWrite(RLED, 0);
	digitalWrite(GLED, 0);
}

// sleep wakeup callback
// it's a hack, don't ask me how it works
void callback() {
	Serial << "z" << endl;
	Serial.flush();
}

// light sleep for desired duration in milliseconds
void sleepCycle(int ms) {
	wifi_set_opmode(NULL_MODE);
	wifi_fpm_set_sleep_type(LIGHT_SLEEP_T);
	wifi_fpm_open();
	wifi_fpm_set_wakeup_cb(callback);
	wifi_fpm_do_sleep(ms * 5000);
	delay(ms + 1);
}

// forces the ESP into Light Sleep for the desired duration in milliseconds
void lightSleep(int ms) {
	//break the sleep up into smaller 5 second cycles
	for (int i = ms; i > 0; i -= 5000) {
		if (i > 5000) {
			// sleep for 1 second
			sleepCycle(5000);
			if(digitalRead(BUTTON)){
				buttonPressed=true;
			}
		} else {
			// sleep for remaining time
			sleepCycle(i);
		}
	}
}

// Battery class for doing battery things
class Battery {
public:
	// Battery charge state stuff
	boolean charged, pluggedIn, batLow;
	double batVoltage, chargeVoltage, chargePercent;

	// constructor
	Battery() {
		this->charged = 0;
		this->pluggedIn = 0;
		this->batLow = 0;
		this->batVoltage = 0;
		this->chargeVoltage = 0;
	}

	// charges the battery if conditions are right for it
	// Turns on the LED if plugged in:
	//		gradient from red to green depending on charge
	//		flashes green when fully charged
	void charge() {
		this->update();
		if (this->pluggedIn) {

			if (this->charged) { // plugged in & charged
				digitalWrite(CHARGE_EN, 1);
				digitalWrite(GLED, 1);
				delay(10);
				LEDOff();

			} else { // plugged in & not charged
				digitalWrite(CHARGE_EN, 1);
				ledGradient(mapd(this->batVoltage, 3.0, 4.16, 0, 255));
			}

		} else {

			// disable charging & turn off LEDs when unplugged
			if (this->charged) { // unplugged & charged
				digitalWrite(CHARGE_EN, 0);
				LEDOff();

			} else { // unplugged & not charged
				digitalWrite(CHARGE_EN, 0);
				LEDOff();
			}

			// lights the red LED if the battery is dying
			if (this->batLow) {
				ledGradient(0);
			}
		}
	}

	//updates charge percentage, battery voltage, charge voltage, charged, plugged-in, & low battery flags
	void update() {

		double bat1 = getBatV(1);
		double bat2 = getChargeV(1);

		// calculates battery charge percent (only relevant when plugged in)
		this->chargePercent = (bat1 - 1016 / 1.4) / (1016 - 1016 / 1.4);

		// detects if the battery is dying (vbat drops below 3.6v) & updates low battery warning flag
		if (bat1 < 863) {
			this->batLow = 1;
		}

		// detects if we're plugged in to USB power & updates the flag
		if ((bat2 >= 995.0) || (this->chargeVoltage >= 1.01*this->batVoltage)) {
			this->pluggedIn = 1;
		} else {
			this->pluggedIn = 0;
		}

		// updates the charged status flag if we're plugged in
		if (this->pluggedIn) {
			if (this->chargePercent >= 0.99) {
				this->charged = 1;
			}
			if (this->chargePercent <= 0.95) {
				this->charged = 0;
			}
		}
	}

	// updates the current battery voltage & returns adc counts if passed a 1, voltage if passed a 0 or nothing
	double getBatV(boolean adc = 0) {
		digitalWrite(CHARGE_EN, 0);
		delay(1);
		double v = this->batReadx(50);
		this->batVoltage = adcToVolts(v);
		if (adc) {
			return v;
		} else {
			return this->batVoltage;
		}
	}

	// updates the current battery charge voltage & returns adc counts if passed a 1, voltage if passed a 0 or nothing
	double getChargeV(boolean adc = 0) {
		digitalWrite(CHARGE_EN, 1);
		delay(1);
		double v = this->batReadx(50);
		digitalWrite(CHARGE_EN, 0);
		this->chargeVoltage = adcToVolts(v);
		if (adc) {
			return v;
		} else {
			return this->chargeVoltage;
		}
	}

	// reads the battery voltage x times and returns the average adc count
	double batReadx(int x) {
		double sum = 0;
		digitalWrite(BAT_CHECK, 1);
		delayMicroseconds(250); // adc settling time
		for (int i = 0; i < x; i++) {
			int bat = analogRead(VBAT_SCALED);
			sum += bat;
			delayMicroseconds(100);
		}
		digitalWrite(BAT_CHECK, 0);
		return sum / x;
	}

};

// class to make wifi objects for connecting really fast to a network
class braceletWifi {
private:
	// WiFi config
	IPAddress ip, gateway, subnet;
	IPAddress dns1 = IPAddress(1, 1, 1, 1);
	IPAddress dns2 = IPAddress(1, 0, 0, 1);
	int channel = 0;
	unsigned char bssid[18];
	bool debug;

public:
	char *ssid, *pass;

	braceletWifi() {}

	braceletWifi(char *SSID, char *PASS, bool DEBUG) {
		this->ssid = SSID;
		this->pass = PASS;
		this->debug = DEBUG;
	}

	// Saves current WiFi configuration for faster repeated connection
	// discards settings if passed a 1
	void config(boolean discard = false) {
		if (discard) { // discard settings
			this->channel = 0;
			this->ip = IPAddress(0, 0, 0, 0);
			this->gateway = IPAddress(0, 0, 0, 0);
			this->subnet = IPAddress(0, 0, 0, 0);
			for (int i = 0; i < 17; i++) {
				this->bssid[i] = 0;
			}
		} else { // save current settings
			this->channel = WiFi.channel();
			this->ip = WiFi.localIP();
			this->gateway = WiFi.gatewayIP();
			this->subnet = WiFi.subnetMask();
			for (int i = 0; i < 17; i++) {
				this->bssid[i] = WiFi.BSSID()[i];
			}
		}
	}

	// function to set up the WiFi & connect
	void connect() {
	retry:
		WiFi.setPhyMode(WIFI_PHY_MODE_11N);
		wifi_set_opmode(STATION_MODE);
		WiFi.persistent(false);

		if (this->debug) {
			Serial.println("");
			Serial.print("Connecting to ");
			Serial.println(this->ssid);
		}

		// connects to the wifi network with as much information as we currently have
		if (this->channel == 0) {
			if (this->debug) {
				Serial.println("unconfigured");
			}
			WiFi.begin(this->ssid, this->pass);
		} else {
			if (this->debug) {
				Serial.println("configured");
			}
			WiFi.config(this->ip, this->gateway, this->subnet, this->dns1, this->dns2);
			WiFi.begin(this->ssid, this->pass, this->channel, this->bssid, 1);
		}

		// waits for connection, retries if it fails
		while (WiFi.status() != WL_CONNECTED) {
			delay(100);
			if (this->debug) {
				Serial.println(WiFi.status());
			}
			if (WiFi.status() == 4) {
				if (this->channel != 0) { // discard configs if we fail to connect with them
					this->config(1);
				}
				wifi_set_opmode(NULL_MODE);
				goto retry;
			}
		}

		if (this->debug) {
			Serial.println();
			Serial.println("WiFi connected");
			Serial.print("IP address: ");
			Serial.println(WiFi.localIP());
			Serial.print("Netmask: ");
			Serial.println(WiFi.subnetMask());
			Serial.print("Gateway: ");
			Serial.println(WiFi.gatewayIP());
			Serial.println();
		}

		// save wifi settings for faster reconnection
		if (channel == 0) {
			this->config();
		}
	}

	// getters
	IPAddress getIP() {
		return this->ip;
	}

	IPAddress getGateway() {
		return this->gateway;
	}

	IPAddress getSubnet() {
		return this->subnet;
	}

	IPAddress getDNS1() {
		return this->dns1;
	}

	IPAddress getDNS2() {
		return this->dns2;
	}

	int getChannel() {
		return this->channel;
	}

	// setters
	void setIP(IPAddress IP) {
		this->ip = IP;
	}

	void setGateway(IPAddress IP) {
		this->gateway = IP;
	}

	void setSubnet(IPAddress IP) {
		this->subnet = IP;
	}

	void setDNS1(IPAddress IP) {
		this->dns1 = IP;
	}

	void setDNS2(IPAddress IP) {
		this->dns2 = IP;
	}

	void setChannel(int cha) {
		this->channel = cha;
	}

};