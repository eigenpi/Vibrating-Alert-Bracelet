To compile and run the server, download the python code and client list and store them to the same directory on 
the device. The device running the server must have python3 installed and be able to function as a wifi access 
point. 

At the moment, all devices must be connected to the network created by the server device to allow for the system 
to function as intended. After setting up the device, the server serial output will display the connections received.

To change the displayed list of clients, the clientList.txt file must be modified and the server needs to be
restarted. In order for the server to recognize the bracelets, the MAC address for the bracelet will need to be known
and inputted. Once the server is restarted, the server will report the newly updated list of clients to any connected apps.

