#include <Adafruit_Sensor_Calibration.h>
#include <Adafruit_AHRS.h>
#include <Adafruit_LSM6DSOX.h>
#include <Adafruit_LIS3MDL.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#if defined(ADAFRUIT_SENSOR_CALIBRATION_USE_EEPROM)
  Adafruit_Sensor_Calibration_EEPROM cal;
#else
  Adafruit_Sensor_Calibration_SDFat cal;
#endif

#define FILTER_UPDATE_RATE_HZ 5
#define PRINT_EVERY_N_UPDATES 10


const char* ssid = "kinani";
const char* password = "password123";
const char* serverName = "http://192.168.148.60:5000/data"; 
Adafruit_Sensor *accelerometer, *gyroscope, *magnetometer;

// pick your filter! slower == better quality output
//Adafruit_NXPSensorFusion filter; // slowest
//Adafruit_Madgwick filter;  // faster than NXP
Adafruit_Mahony filter;  // fastest/smalleset

Adafruit_LIS3MDL lis3mdl;
Adafruit_LSM6DSOX lsm6ds;

bool init_sensors(void) {
  if (!lsm6ds.begin_I2C() || !lis3mdl.begin_I2C()) {
    return false;
  }
  accelerometer = lsm6ds.getAccelerometerSensor();
  gyroscope = lsm6ds.getGyroSensor();
  magnetometer = &lis3mdl;

  return true;
}

void setup_sensors(void) {
  // set lowest range
  lsm6ds.setAccelRange(LSM6DS_ACCEL_RANGE_2_G);
  lsm6ds.setGyroRange(LSM6DS_GYRO_RANGE_250_DPS);
  lis3mdl.setRange(LIS3MDL_RANGE_4_GAUSS);

  // set slightly above refresh rate
  lsm6ds.setAccelDataRate(LSM6DS_RATE_104_HZ);
  lsm6ds.setGyroDataRate(LSM6DS_RATE_104_HZ);
  lis3mdl.setDataRate(LIS3MDL_DATARATE_1000_HZ);
  lis3mdl.setPerformanceMode(LIS3MDL_MEDIUMMODE);
  lis3mdl.setOperationMode(LIS3MDL_CONTINUOUSMODE);
}

uint32_t timestamp;

void setup() {
  Serial.begin(115200);
  while (!Serial) yield();

  if (!cal.begin()) {
    Serial.println("Failed to initialize calibration helper");
  } else if (! cal.loadCalibration()) {
    Serial.println("No calibration loaded/found");
  }

  if (!init_sensors()) {
    Serial.println("Failed to find sensors");
    while (1) delay(10);
  }
  
  accelerometer->printSensorDetails();
  gyroscope->printSensorDetails();
  magnetometer->printSensorDetails();

  setup_sensors();
  filter.begin(FILTER_UPDATE_RATE_HZ);
  timestamp = millis();

  Wire.setClock(400000); // 400KHz

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}


void loop() {
  float roll, pitch, yaw;
  float gx, gy, gz;
  static uint8_t counter = 0;

  if ((millis() - timestamp) < (1000 / FILTER_UPDATE_RATE_HZ)) {
    return;
  }
  timestamp = millis();
  // Read the motion sensors
  sensors_event_t accel, gyro, mag;
  accelerometer->getEvent(&accel);
  gyroscope->getEvent(&gyro);
  magnetometer->getEvent(&mag);

  cal.calibrate(mag);
  cal.calibrate(accel);
  cal.calibrate(gyro);
  // Gyroscope needs to be converted from Rad/s to Degree/s
  // the rest are not unit-important
  gx = gyro.gyro.x * SENSORS_RADS_TO_DPS;
  gy = gyro.gyro.y * SENSORS_RADS_TO_DPS;
  gz = gyro.gyro.z * SENSORS_RADS_TO_DPS;

  // Update the SensorFusion filter
  filter.update(gx, gy, gz, 
                accel.acceleration.x, accel.acceleration.y, accel.acceleration.z, 
                mag.magnetic.x, mag.magnetic.y, mag.magnetic.z);

  roll = filter.getRoll();
  pitch = filter.getPitch();
  yaw = filter.getYaw();

  Serial.println("IMU Data");
  // Serial.println("Roll: "+String(roll));
  Serial.println("Pitch: "+String(pitch));
  Serial.println("Yaw: "+String(yaw));

  if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      http.begin(serverName);

      http.addHeader("Content-Type", "application/json");
      DynamicJsonDocument doc(1024);
      doc["roll"] = roll;
      doc["pitch"] = pitch;
      doc["yaw"] = yaw;
      String imu_data;
      serializeJson(doc, imu_data);
      Serial.println("Sending... IMU Data");
      int httpResponseCode = http.POST(imu_data);
      Serial.println("HTTP Response Code: " + httpResponseCode);
      http.end();
  }
  Serial.print("Took "); Serial.print(millis()-timestamp); Serial.println(" ms");
  // delay(500); 
}