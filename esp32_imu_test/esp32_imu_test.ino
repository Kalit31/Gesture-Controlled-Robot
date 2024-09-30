#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_LSM6DSOX.h>

// Replace with your network credentials
const char* ssid = "Doritos";
const char* password = "GTPDEastPoint@1346";

// Create an instance of the server
WiFiServer server(80);
Adafruit_LSM6DSOX lsm6dsox;
WiFiClient client;
unsigned long previousMillis = 0;
const long interval = 2000;

void setup() {
  Serial.begin(115200);
  
  // Initialize the IMU
  if (!lsm6dsox.begin_I2C()) {
    Serial.println("Failed to find LSM6DSOX chip");
    while (1) { delay(10); }
  }


  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.print("Connected to WiFi. IP address: ");
  Serial.println(WiFi.localIP());

  // Define a route for the root URL
  // server.on("/", []() {
  //   server.send(200, "text/html", "<h1>Hello, ESP32!</h1>");
  // });

  // Start the server
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  // Check for a client connection
  if (!client.connected()) {
    client = server.available();
    if (client) {
      Serial.println("New client connected");
      // Send initial HTML response
      sendResponse();
    }
  } else {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      sendResponse();  // Update data every 'interval' milliseconds
    }
  }

  // Handle client disconnection
  if (!client.connected()) {
    Serial.println("Client disconnected");
    client.stop();
  }
}

void sendResponse() {
    sensors_event_t accel, gyro, temp;
    lsm6dsox.getEvent(&accel, &gyro, &temp);
      
    String jsonResponse = String("{") +
      "\"acceleration\":{\"x\":" + String(accel.acceleration.x) +
      ",\"y\":" + String(accel.acceleration.y) +
      ",\"z\":" + String(accel.acceleration.z) + "}," +
      "\"gyroscope\":{\"x\":" + String(gyro.gyro.x) +
      ",\"y\":" + String(gyro.gyro.y) +
      ",\"z\":" + String(gyro.gyro.z) + "}" +
      "}";
      
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: application/json");
    client.println("Connection: close");
    client.println();
    client.print(jsonResponse);
}