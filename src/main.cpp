// ------- GSM/GPRS/GPS Setup -------

#include <Arduino.h>
#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>

static const int RXPin = 0, TXPin = 1;
String baseUrl = "www.google.com/maps/dir/";
String smsMessage;
unsigned long interval = 10000;
static const uint32_t GPSBaud = 9600;
unsigned long previousMillis = 0;

TinyGPSPlus gps;
SoftwareSerial ss(RXPin, TXPin);

void setup() {
	Serial.begin(9600);
	ss.begin(GPSBaud);
	
	Serial.println("Starting...");
	ss.println("AT\r");             // check communication with the module
	delay(100);
	ss.println("AT+GPS=1\r");       // turn on the GPS
	delay(100);
	ss.println("AT+CREG=2\r");      // configure the network registration to provide more detailed information
	delay(6000);
	ss.println("AT+CGATT=1\r");     // attach to GPRS service
	delay(6000);
	ss.println("AT+CGDCONT=1,\"IP\",\"WWW\"\r");  // set the APN for the GPRS context
  	delay(6000);
  	ss.println("AT+CGACT=1,1\r");   // activate the GPRS context
  	delay(6000);
 	ss.println("AT+GPS=1\r");       // turn on the GPS (again)
  	delay(1000);
  	ss.println("AT+GPSRD=10\r");    // start continuous GPS reading every 10 seconds
  	delay(100);
  	ss.println("AT+CMGF=1\r");      // set the SMS mode to text
  	delay(1000);

  	Serial.println("Setup Executed");
}

void loop() {
	smartDelay(2000);

  	if (millis() > 5000 && gps.charsProcessed() < 10)
    Serial.println(F("No GPS data received: check wiring"));

  	unsigned long currentMillis = millis();

  	if ((unsigned long)(currentMillis - previousMillis) >= interval) {
		send_gps_data();
		previousMillis = currentMillis;
  	}
}

static void smartDelay(unsigned long ms) {
	unsigned long start = millis();
	do {
		while (ss.available())
      	gps.encode(ss.read());
	} while (millis() - start < ms);
}

void send_gps_data() {
	if (gps.location.lat() == 0 || gps.location.lng() == 0) {
		Serial.println("Return Executed");
		return;
	}
	
	float latitude = gps.location.lat();
	float longitude = gps.location.lng();
	
	Serial.print("Latitude (deg): ");
	Serial.println(latitude);
	Serial.print("Longitude (deg): ");
	Serial.println(longitude);
	
	String locationUrl = baseUrl + String(latitude, 6) + "," + String(longitude, 6) + "/";
	smsMessage = "GAWAT!!!\n\n"
               "Seseorang membutuhkan pertolongan darurat! Cek lokasi terkini di bawah ini:\n" +
               locationUrl + "\n\n" +
               "Latitude (deg): " + String(latitude, 2) + "\n" +
               "Longitude (deg): " + String(longitude, 2);

  	Serial.println("Sending Message");

  	ss.println("AT+CMGF=1\r");
  	delay(1000);

  	ss.print("AT+CMGS=\"+6285801334968\"\r"); // Replace with your mobile number
  	delay(1000);
  	ss.print(smsMessage);
  	ss.write(0x1A);
  	delay(1000);
}
