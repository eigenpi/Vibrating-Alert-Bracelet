BatteryDrainTest.ino
ReadMe by Jason Reichard
Team E45 - Marquette University
April 26, 2019

This program is used to test the lifespan of a single battery charge by having the VAB upload the battery voltage & the
current value of millis() to a google sheet. The program runs until the battery is completely drained. millis() is used to detect whether
or not the bracelet has reset itself during the test. The program also features the ability to recharge the battery when plugged into USB power.

To use this program you must do the following:
	
	Create a google sheet

	go to tools -> script editor

	Paste the code below:

		function doGet(e) {
		var mo = e.parameter.func;
			if(mo == "addData") {
				var stat = add_data(e);
				if(stat == 1) {
					var result = {status : true};
					return ContentService.createTextOutput(JSON.stringify(result)).setMimeType(ContentService.MimeType.JSON);
				}
			}
		}

		function add_data(e) {
			var sheet = SpreadsheetApp.openByUrl('YourGoogleSheetURLHere');
  
			var CurrentDate = new Date();
			var Date_ = Utilities.formatDate(CurrentDate, "GMT-05:00", "MM/dd/YYYY");
			var Time_ = Utilities.formatDate(CurrentDate, "GMT-05:00", "HH:mm:ss");

			sheet.appendRow([Date_, Time_, e.parameter.millis, e.parameter.batV]);
			return 1;
		}

	Replace the Google sheet URL with your own & edit the date & time formats as you see fit

	Click Publish -> Deploy as a web app

	Copy the text in the web app URL between "https://script.google.com/macros/s/" and "/exec" This is your ScriptID

	Paste your ScriptID into the program

	Edit the SSID & password to match those of your WiFi network

	Save, compile, & upload to the ESP8266

	Data will start appearing in your google sheet if everything was done correctly.