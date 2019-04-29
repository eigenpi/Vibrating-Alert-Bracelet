This project was created using the Arduino IDE

This .ino file contains the code necessary for the functonality of the bracelet. This program requires the BraceletHardware.h header file in order to compile. There are also two additional libraries that need to be added for the operation. The first is the ESP8266 library version 2.4.0-rc2 in order for the light sleep mode to function. The next necessary library is Streaming which is necessary for the header file.

Once the file is opened a few changes need to be made for the code to function depending on the setup. The ssid, password, and host IP address need to be changed to match your current system. Once these changes have been made the board must be changed by going to Tools>Board and then you can select either generic ESP8266 module or Adafruit HUZZAH ESP8266. If one does not compile try the other.

Serial monitoring is enabled by default but can be turned off by adding the arguement of 1 to hwInit() call in the setup.