BraceletHardware.h
ReadMe by Jason Reichard
Team E45 - Marquette University
April 26, 2019

This header file is created to provide easy access to the Vibrating Alert Bracelet's Hardware. The function bodies are included in the header to 
provide full visibility. 

It provides two classes:
	A Battery class to manage the current state of the battery, the battery charger, and the state of the bi-color LED 
	in response to battery charge level.

		To use the battery simply initialize a new Battery object with the default constructor. The state of the battery will be updated 
		upon calling Battery.update(). The state of the battery charger & charge-dependent LED lighting will be updated upon calling
		Battery.charge(). Note that Battery.charge() also calls Battery.update().

	A BraceletWifi class to manage the wifi connection of the bracelet. This class was built simply as a convenience to connect and reconnect
	to a WiFi network as quickly as possible when used in conjunction with light sleep.

		To use the wifi class initialize a new BraceletWifi object with your SSID and password.

		Calling BraceletWifi.connect() will attempt to connect to the WiFi network as fast as possible. After connecting to the network for the
		first time the configuration settings of the network are remembered to facilitate a much faster reconnection awterward.

It also provides pin definitions, a hardware initialization function, and several other utility functions including a working timed Light Sleep 
implementation.

As of 4/26/2019 the only way to make light sleep work is to be using the ESP8266 core version 2.4.0-rc1 or 2.4.0-rc2.
