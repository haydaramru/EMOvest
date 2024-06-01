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

void makeCall() {
	Serial.println("Making a call...");
	A9G.print("ATD+6285801334968;\r"); // Replace phone number
	delay(1000);
	Serial.println("Call initiated.");
}

// ------- HC-SR04 Setup -------
const int trigPin = 2;
const int echoPin = 4;
long duration, distance;

// Message sending control variables
int messageCount = 0;
bool startSending = false;

void setup() {
	Serial.begin(115200);
	A9G.begin(GPSBaud);
	
	// HC-SR04 Init
	pinMode(trigPin, OUTPUT);
	pinMode(echoPin, INPUT);
	
	// Wi-Fi Init
	WiFi.begin(ssid, password);
  	Serial.println("Connecting WiFi...");
  	while(WiFi.status() != WL_CONNECTED) {
		delay(500);
    	Serial.print(".");
	}
  	Serial.println("");
  	Serial.print("Connected to WiFi network with IP Address: ");
  	Serial.println(WiFi.localIP());

	// A9G Init
  	Serial.println("Initialize A9G...");
  	delay(10000);
  	sendCommand("AT\r", 1000);								// check communication with the module
  	sendCommand("AT+CGATT=1\r", 10000);						// attach to GPRS service
  	sendCommand("AT+CGDCONT=1,\"IP\",\"WWW\"\r", 4000);		// set the APN for the GPRS context
  	sendCommand("AT+CGACT=1,1\r", 5000);					// activate the GPRS context
  	sendCommand("AT+GPS=1\r", 4000);						// turn on the GPS
  	sendCommand("AT+GPSRD=10\r", 5000);						// start continuous GPS reading every 10 seconds
  	sendCommand("AT+CMGF=1\r", 1000);						// set the SMS mode to text

  	Serial.println("Setup Executed.");
}

void loop() {
	// Read distance from HC-SR04
	digitalWrite(trigPin, LOW);
	delayMicroseconds(4);
	digitalWrite(trigPin, HIGH);
	delayMicroseconds(10);
	digitalWrite(trigPin, LOW);
  	duration = pulseIn(echoPin, HIGH);
	distance = duration * 0.034 / 2;
	Serial.print("Distance: ");
  	Serial.print(distance);
  	Serial.println(" cm");

	unsigned long currentMillis = millis();

	// Check if distance is less than 20 cm
  	if (distance < 20) {
		startSending = true;
		messageCount = 0; // Reset message count when the condition is first met
  	}

	if (startSending && messageCount < 5 && (currentMillis - previousMillis >= interval)) {
		smartDelay(2000);
		if (millis() > 5000 && gps.charsProcessed() < 10) {
			Serial.println(F("No GPS data received: check wiring"));
		}
		
		prepareGPSPayload();
		sendSMS();
		sendWhatsApp(waMessage);
		makeCall();

		previousMillis = currentMillis;
		messageCount++;
		if (messageCount >= 5) {	// Stop sending after 5 messages
			startSending = false;
		}
	}
}

void sendCommand(String command, unsigned long waitTime) {	// send AT command to A9G module
	A9G.println(command);
	delay(waitTime);
	while (A9G.available()) {
		Serial.write(A9G.read());
	}
}
