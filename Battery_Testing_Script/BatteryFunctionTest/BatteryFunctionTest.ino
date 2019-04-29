/*
	Name:       Battery Testing.ino
	Created:	3/7/2019 4:07:23 PM
	Author:     Jason Reichard

	This is a short debug program for testing the detection of various aspects of the battery.
*/

#include "BraceletHardware.h"
Battery myBattery;

void setup() {
	hwInit();
}

void loop() {
	myBattery.charge();
	
	//debugging
	Serial.print("State of Charge: ");
	Serial.println(myBattery.chargePercent);

	Serial.print("Battery Voltage: ");
	Serial.println(myBattery.batVoltage);

	Serial.print("Charge Voltage: ");
	Serial.println(myBattery.chargeVoltage);

	Serial.print("Plugged in: ");
	Serial.println(myBattery.pluggedIn);

	Serial.print("Charge Complete: ");
	Serial.println(myBattery.charged);

	Serial.print("Low Battery Warning: ");
	Serial.println(myBattery.batLow);

	delay(1000);
}