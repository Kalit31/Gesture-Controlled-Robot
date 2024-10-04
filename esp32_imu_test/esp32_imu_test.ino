#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_LSM6DSOX.h>
#include <Adafruit_LIS3MDL.h>

// Replace with your network credentials
const char* ssid = "Doritos";
const char* password = "GTPDEastPoint@1346";

// Create an instance of the server
WiFiServer server(80);
Adafruit_LSM6DSOX lsm6dsox;
Adafruit_LIS3MDL lis3mdl;    // LIS3MDL object for magnetometer
WiFiClient client;
unsigned long previousMillis = 0;
const long interval = 2000;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
        ; // Wait for serial port to connect. Needed for native USB.
  }
  Serial.println("Initialized!");

  // Initialize the IMU
  if (!lsm6dsox.begin_I2C()) {
    Serial.println("Failed to find LSM6DSOX chip");
    while (1) { delay(10); }
  }

  // Initialize LIS3MDL (Magnetometer)
  if (!lis3mdl.begin_I2C()) {
    Serial.println("Couldn't find LIS3MDL sensor.");
    while (1);
  }
  Serial.println("LIS3MDL found!");
}

void loop() {
  sensors_event_t accel, gyro, temp;
  lsm6dsox.getEvent(&accel, &gyro, &temp);
    
  Serial.print(accel.acceleration.x); Serial.print(",");
  Serial.print(accel.acceleration.y); Serial.print(",");
  Serial.print(accel.acceleration.z); Serial.print(",");  

  Serial.print(gyro.gyro.x); Serial.print(",");
  Serial.print(gyro.gyro.y); Serial.print(",");
  Serial.print(gyro.gyro.z); Serial.print(",");


  sensors_event_t mag_event;
  lis3mdl.getEvent(&mag_event);
  Serial.print(mag_event.magnetic.x); Serial.print(",");
  Serial.print(mag_event.magnetic.y); Serial.print(",");
  Serial.println(mag_event.magnetic.z); 

  delay(200);
}
