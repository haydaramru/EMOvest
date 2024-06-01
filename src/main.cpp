// TODO
/*
- voice call
- tele
- wa api
- Implement HC-SR04
*/

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <UrlEncode.h>
#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>

// ------- Wi-Fi Setup -------
const char* ssid = "Hidden Network";	// Wi-Fi Name
const char* password = "qwertyuiop";	// Wi-Fi Password

// ------- WhatsApp API Setup -------
String waMessage;
String waNumber = "+6285801334968";			// Replace WhatsApp Number
String apiKey = "3063201";

void sendWhatsApp(String message){
	String url = "https://api.callmebot.com/whatsapp.php?phone=" + waNumber + "&apikey=" + apiKey + "&text=" + urlEncode(message);    
  	HTTPClient http;
  	http.begin(url);
  	http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  	int httpResponseCode = http.POST(url);
  	if (httpResponseCode == 200){
		Serial.print("Message sent successfully");
  	} else {
		Serial.println("Error sending the message");
		Serial.print("HTTP response code: ");
		Serial.println(httpResponseCode);
  	}
  	http.end();
}

// ------- GSM/GPRS/GPS Setup -------
static const int RXPin = 0, TXPin = 1;
String baseUrl = "www.google.com/maps/dir/";
String smsMessage;
unsigned long interval = 10000;			// interval delay to send SMS and WhatsApp
static const uint32_t GPSBaud = 115200;
unsigned long previousMillis = 0;

TinyGPSPlus gps;
SoftwareSerial A9G(RXPin, TXPin);

static void smartDelay(unsigned long ms) {
	unsigned long start = millis();
	do {
		while (A9G.available())
      	gps.encode(A9G.read());
	} while (millis() - start < ms);
}

void prepareGPSPayload() {
	float latitude = gps.location.lat();
	float longitude = gps.location.lng();

	if (latitude == 0 || longitude == 0) {
		Serial.println("Return Executed");
		return;
	}
	
	Serial.print("Latitude (deg): ");
	Serial.println(latitude);
	Serial.print("Longitude (deg): ");
	Serial.println(longitude);

	String locationUrl = baseUrl + String(latitude, 6) + "," + String(longitude, 6) + "/";
	smsMessage = "GAWAT!!!\n\n"
            	 "Seseorang membutuhkan pertolongan darurat! Cek lokasi terkini di bawah ini:\n" + locationUrl + "\n\n" +
            	 "Latitude (deg): " + String(latitude, 2) + "\n" +
            	 "Longitude (deg): " + String(longitude, 2);
	waMessage = "GAWAT!!!\n\nSeseorang membutuhkan pertolongan darurat! Cek lokasi terkini di bawah ini:\n" + locationUrl;
}

void sendSMS() {
  	Serial.println("Sending SMS Message..");
  	A9G.print("AT+CMGS=\"+6285801334968\"\r"); // Replace phone number
  	delay(1000);
  	A9G.print(smsMessage);
  	delay(200);
	A9G.write(0x1A);	// ASCII code for CTRL+Z
  	delay(1000);
	Serial.println("SMS Message Sent.");
}

void setup() {
	Serial.begin(115200);
	A9G.begin(GPSBaud);
	
	Serial.println("Starting...");
	delay(10000);
	A9G.println("AT\r");             // check communication with the module
	delay(1000);
	A9G.println("AT+CGATT=1\r");     // attach to GPRS service
	delay(10000);
	A9G.println("AT+CGDCONT=1,\"IP\",\"WWW\"\r");  // set the APN for the GPRS context
  	delay(4000);
  	A9G.println("AT+CGACT=1,1\r");   // activate the GPRS context
  	delay(5000);
	A9G.println("AT+GPS=1\r");       // turn on the GPS
	delay(4000);
  	A9G.println("AT+GPSRD=10\r");    // start continuous GPS reading every 10 seconds
  	delay(5000);
  	A9G.println("AT+CMGF=1\r");      // set the SMS mode to text
  	delay(1000);

  	Serial.println("Setup Executed.");
}

void loop() {
	smartDelay(2000);
  	if (millis() > 5000 && gps.charsProcessed() < 10) {
		Serial.println(F("No GPS data received: check wiring"));
	}

	prepareGPSPayload();

  	unsigned long currentMillis = millis();

  	if ((unsigned long)(currentMillis - previousMillis) >= interval) {
		sendSMS();
		sendWhatsApp(waMessage);
		previousMillis = currentMillis;
  	}
}
