BatteryFunctionTest.ino
ReadMe by Jason Reichard
Team E45 - Marquette University
April 26, 2019

This is a short test progam whose sole purpose was to refine the functionality of the Battery class found in BraceletHardware.h 

The testing was necessary in order to ensure that the detection of USB power was reliable. Due to the lack of GPIO pins, the detection of USB 
power is done via reading the battery voltage, enabling the charger, and reading it again. A comparison is then done on the result of the two
measurements in order to determine whether the bracelet is plugged in.

The program will continuously print data about the current state of the battery:
	
	Battery Voltage

	Charger Voltage

	State of Charge

	Plugged-In flag

	Charged flag
	
	Low Battery Warning flag