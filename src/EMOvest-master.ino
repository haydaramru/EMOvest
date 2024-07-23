/****************************************************************************************************************
This is the MASTER Code for PKM KC Emo-Vest
Consist of :
Accelerometer
Relay Activation
LED Blink
Speaker Activation

Gamma Nasim 07/07/2024
gammanasim@gmail.com

MASTER :
pin 7 relay
pin 8 speaker
pin 13 LED
pin 4 SDA
pin 5 SCL

All works on 5V
Flowchart = https://bit.ly/4bDgkP8
*****************************************************************************************************************/
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#define NOTE_E4  330

Adafruit_MPU6050 mpu;
float kecepatanJatuh;
float previousKecepatanJatuh = 0;
bool jatuhMaster = false;
bool jatuhSlave = false;
bool penggunaJatuh = false;
bool relayActivated = false; // Track relay activation
int readCount = 0;
const int relayPin = 7; // Define the pin connected to the relay
const int ledPin = 13;  // Define the pin connected to the LED
int telfon = 0;
unsigned long startMillis;
unsigned long ignoreDuration = 5000;
bool accelerometerProcessed = false; // Flag to indicate if the accelerometer has been processed

void setup(void) {
  Serial.begin(115200);
  while (!Serial)
    delay(10); // will pause Zero, Leonardo, etc until serial console opens

  pinMode(relayPin, OUTPUT); // Set the relay pin as output
  digitalWrite(relayPin, HIGH); // Ensure the relay is off initially
  pinMode(8, OUTPUT); // Set the pin for tone output as output
  pinMode(ledPin, OUTPUT); // Set the LED pin as output

  Serial.println("Adafruit MPU6050 test!");

  // Try to initialize!
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  Serial.print("Accelerometer range set to: ");
  switch (mpu.getAccelerometerRange()) {
  case MPU6050_RANGE_2_G:
    Serial.println("+-2G");
    break;
  case MPU6050_RANGE_4_G:
    Serial.println("+-4G");
    break;
  case MPU6050_RANGE_8_G:
    Serial.println("+-8G");
    break;
  case MPU6050_RANGE_16_G:
    Serial.println("+-16G");
    break;
  }
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  Serial.print("Gyro range set to: ");
  switch (mpu.getGyroRange()) {
  case MPU6050_RANGE_250_DEG:
    Serial.println("+- 250 deg/s");
    break;
  case MPU6050_RANGE_500_DEG:
    Serial.println("+- 500 deg/s");
    break;
  case MPU6050_RANGE_1000_DEG:
    Serial.println("+- 1000 deg/s");
    break;
  case MPU6050_RANGE_2000_DEG:
    Serial.println("+- 2000 deg/s");
    break;
  }

  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  Serial.print("Filter bandwidth set to: ");
  switch (mpu.getFilterBandwidth()) {
  case MPU6050_BAND_260_HZ:
    Serial.println("260 Hz");
    break;
  case MPU6050_BAND_184_HZ:
    Serial.println("184 Hz");
    break;
  case MPU6050_BAND_94_HZ:
    Serial.println("94 Hz");
    break;
  case MPU6050_BAND_44_HZ:
    Serial.println("44 Hz");
    break;
  case MPU6050_BAND_21_HZ:
    Serial.println("21 Hz");
    break;
  case MPU6050_BAND_10_HZ:
    Serial.println("10 Hz");
    break;
  case MPU6050_BAND_5_HZ:
    Serial.println("5 Hz");
    break;
  }

  Serial.println("");
  startMillis = millis();
  delay(100);
}

void loop() {
  // Get new sensor events with the readings
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  previousKecepatanJatuh = kecepatanJatuh;
  kecepatanJatuh = a.acceleration.z;

  // Ignore the first 5 readings
  if (readCount < 5) {
    readCount++;
  } else {
    // Check if the difference between the previous and current kecepatanJatuh is greater than 7
    if (abs(kecepatanJatuh - previousKecepatanJatuh) > 7) {
      jatuhMaster = true;
    }
  }

  if (Serial.available()) {
    int receivedValue = Serial.read();
    if (receivedValue == 4) {
      telfon = 4;
    } else {
      jatuhSlave = receivedValue;
    }
  }

  if (jatuhMaster && jatuhSlave && !relayActivated) {
    jatuhMaster = false; // Reset the jatuhMaster to avoid repeated messages
    penggunaJatuh = true;
    Serial.write(penggunaJatuh);
    relayActivated = true; // Mark relay as activated
  }

  if (penggunaJatuh) {
    digitalWrite(relayPin, LOW); // Activate the relay
    delay(2000); // Wait for 2 seconds
    digitalWrite(relayPin, HIGH); // Deactivate the relay
    penggunaJatuh = false; // Reset penggunaJatuh
    accelerometerProcessed = true; // Mark accelerometer as processed
  }

  // If telfon == 4, play the tone and blink the LED continuously
  if (telfon == 4) {
    tone(8, NOTE_E4);  // Play the note E4 on pin 8
    digitalWrite(ledPin, HIGH);   // Turn the LED on
    delay(50);        // Play the note for 50 milliseconds
    noTone(8);        // Stop playing the note
    digitalWrite(ledPin, LOW);    // Turn the LED off
    delay(50);        // Wait for 50 milliseconds before playing the note and blinking the LED again
  }

  delay(100); // Add a delay to control the loop speed
}
