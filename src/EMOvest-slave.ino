/****************************************************************************************************************
This is the Slave Code for PKM KC Emo-Vest
Consist of :
Accelerometer
Vibration Sensor

Gamma Nasim 07/07/2024
gammanasim@gmail.com

Slave :
pin 4 SDA
pin 5 SCL
pin 2, 3 vibro sensor

All works on 5V
Flowchart = https://bit.ly/4bDgkP8
*****************************************************************************************************************/

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include "I2Cdev.h"
#include "MPU6050.h"

// MPU6050 instance
Adafruit_MPU6050 mpu;

// Variables for fall detection
float kecepatanJatuh;
float previousKecepatanJatuh = 0;
bool jatuhSlave = false;
bool penggunaJatuh = false;
int readCount = 0;
int telfon = 0;
bool accelerometerProcessed = false; // Flag to indicate if the accelerometer has been processed

// Vibration sensor pins
int vibrationInPin1 = 2;  // First vibration sensor pin
int vibrationInPin2 = 3;  // Second vibration sensor pin
int ledPin = 13;          // LED pin to indicate epilepsi status
bool getaran1 = false;
bool getaran2 = false;
bool epilepsi = false;

unsigned long previousMillis = 0;
const long interval = 3000; // 3 seconds
int vibrationCount1 = 0;
int vibrationCount2 = 0;

void setup(void) {
  Serial.begin(115200);
  while (!Serial)
    delay(10); // will pause Zero, Leonardo, etc until serial console opens

  Serial.println("Adafruit MPU6050 test!");

  // Try to initialize MPU6050
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
  delay(100);

  // Initialize vibration sensor pins
  pinMode(vibrationInPin1, INPUT); 
  pinMode(vibrationInPin2, INPUT); 
  pinMode(ledPin, OUTPUT); // Set LED pin as output
}

void loop() {
  if (penggunaJatuh) {
    // Run the vibration detection code
    unsigned long currentMillis = millis();
  
    // Check first vibration sensor
    if (digitalRead(vibrationInPin1) == HIGH) {
      getaran1 = true;
      vibrationCount1++;
      delay(50); // Delay for stability
    }

    // Check second vibration sensor
    if (digitalRead(vibrationInPin2) == HIGH) {
      getaran2 = true;
      vibrationCount2++;
      delay(50); // Delay for stability
    }

    // Check if both vibrations are detected within the interval
    if (currentMillis - previousMillis >= interval) {
      // Check if both sensors detected vibration 15 times within 3 seconds
      if (vibrationCount1 >= 10 && vibrationCount2 >= 10) {
        epilepsi = true;
        digitalWrite(ledPin, HIGH); // Turn on LED to indicate epilepsi status
        Serial.println("Epilepsi detected");
        telfon = 4;
        Serial.print(telfon);
        Serial.write(telfon);
      } else {
        epilepsi = false;
        digitalWrite(ledPin, LOW); // Turn off LED
      }
    
      // Reset counters and timer
      previousMillis = currentMillis;
      vibrationCount1 = 0;
      vibrationCount2 = 0;
    }
  } else {
    // Run the accelerometer code
    if (accelerometerProcessed) {
      return; // Exit the loop if accelerometer has already been processed
    }

    // MPU6050 sensor event variables
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    previousKecepatanJatuh = kecepatanJatuh;
    kecepatanJatuh = a.acceleration.z;

    // Ignore the first 5 readings for fall detection
    if (readCount < 5) {
      readCount++;
    } else {
      // Check if the difference between the previous and current kecepatanJatuh is greater than 7
      if (abs(kecepatanJatuh - previousKecepatanJatuh) > 7) {
        jatuhSlave = true;
        Serial.write(jatuhSlave);  // Transmit jatuhSlave status
      }
    }

    if (Serial.available()) {
      penggunaJatuh = Serial.read();
      Serial.print(penggunaJatuh);
      if (penggunaJatuh) {
        accelerometerProcessed = true; // Mark accelerometer as processed
      }
    }
  }

  delay(50); // Adjust delay as needed
}
