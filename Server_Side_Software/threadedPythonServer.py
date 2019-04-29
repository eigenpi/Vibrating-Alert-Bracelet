import socket
from _thread import *
import threading
import csv
from datetime import datetime

alertState = 0
clientFields = 2
clientList = []


# thread function
def threaded(c):
    global alertState
    data = c.recv(1024)
    if not data:
        print('Bye')

    data = data.decode('UTF-8')
    # print(data)

    if data == "app":
        data = c.recv(1024)

        data = data.decode('UTF-8')
        if data == "SENDALERT":
            sendString = "alert received"
            c.send(sendString.encode())
            print("APP ALERT RECEIVED")
            alertState = 1
            # This will eventually need to be specific to MAC addresses
        elif data == "CANCELALERT":
            sendString = "alert canceled"
            c.send(sendString.encode())
            print("APP ALERT CANCELED")
            alertState = 0
        elif data == "UPDATE":
            print("APP UPDATE REQUESTED")

            currentTime = str(datetime.now().time())
            currentTime = currentTime.split(":")
            hour = int(currentTime[0])
            minute = int(currentTime[1])
            totalMinuteCount = (hour*60)+minute

            clientString = ""
            for client in clientList:

                clientTime = client.lastCheckInTime
                clientTime = clientTime.split(":")
                hour = int(clientTime[0])
                minute = int(clientTime[1])
                totalClientMinutes = (hour * 60) + minute

                if client.braceletAlert == 1:
                    if totalMinuteCount - client.braceletAlertTime > 5:
                        client.braceletAlert = 0;

                if client.braceletAlert == 1:
                    connectionStatus = "ALERT"
                elif (totalMinuteCount - totalClientMinutes) < 3:
                    connectionStatus = "CONNECTED"
                else:
                    connectionStatus = "DISCONNECTED"

                clientString = clientString + str(client.name) + "," + str(client.room) + "," + str(connectionStatus) + ","
            c.send(clientString.encode())

    elif data == "esp":
        sendString = "hello bracelet"
        c.send(sendString.encode())
        data = c.recv(1024)

        data = data.decode('UTF-8')
        # print(data)
        data = data.split(":")
        braceletAlertState = data[1]
        if data[0] == "CHECK IN":
            print("BRACELET CHECKIN")
            if alertState == 1:
                sendString = "ALERT";
                c.send(sendString.encode())
            else:
                sendString = "NO ALERT";
                c.send(sendString.encode())

            # Receive MAC addresses from the bracelet at this point
            data = c.recv(1024)
            data = data.decode('UTF-8')
            # print(data)
            # This checks the MAC address of the bracelet against the list of clients and updates the corresponding time
            for client in clientList:
                if client.macAddress == data:
                    if braceletAlertState == "1":
                        client.braceletAlert = 1
                        alertTime = str(datetime.now().time())
                        alertTime = alertTime.split(":")
                        hour = int(alertTime[0])
                        minute = int(alertTime[1])
                        client.braceletAlertTime = (hour * 60) + minute
                    client.lastCheckInTime = str(datetime.now().time())

    # connection closed
    c.close()


class Client(object):
    name = ""
    room = ""
    macAddress = ""
    lastCheckInTime = ""
    braceletAlert = 0       #This is used to alert the app from the bracelet
    braceletAlertTime = ""
    appAlert = 0            #This is used to buzz the bracelet from the app
    appAlertTime = ""
    # connectionStatus = "CONNECTED"

    def __init__(self, name, room, macAddress, time, alertTime):
        self.name = name
        self.room = room
        self.macAddress = macAddress
        self.lastCheckInTime = time
        self.braceletAlertTime = alertTime


def createClient(name, room, macAddress, time, alertTime):
    client = Client(name, room, macAddress, time, alertTime)
    return client


def Main():
    host = ""
    port = 43000

    global clientData
    with open('clientList.txt', 'r') as f:
        reader = csv.reader(f)
        clientRawData = list(reader)

    tempName = ""
    tempMACAddress = ""
    # currentTime = str(datetime.now().time())
    currentTime = "0:0"
    for i in range(1, len(clientRawData)):
        tempName = clientRawData[i][0]
        tempRoom = clientRawData[i][1]
        tempMACAddress = clientRawData[i][2]
        clientList.append(createClient(tempName, tempRoom, tempMACAddress, currentTime, currentTime))

    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.bind((host, port))
    #print("socket binded to post", port)

    # put the socket into listening mode
    s.listen(5)
    print("socket is listening")

    # a forever loop until client wants to exit
    while True:
        # establish connection with client
        c, addr = s.accept()

        # print('Connected to :', addr[0], ':', addr[1])

        # Start a new thread and return its identifier
        start_new_thread(threaded, (c,))
    s.close()


if __name__ == '__main__':
    Main()