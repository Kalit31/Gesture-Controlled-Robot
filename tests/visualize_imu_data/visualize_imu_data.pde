import processing.serial.*;

Serial myPort;  // The Serial port
String incomingData;
float[] sensorData = new float[9];  // Array to store accelerometer, gyroscope, and magnetometer data

void setup() {
  size(800, 600, P3D);
  background(0);

  // Initialize serial communication  
  myPort = new Serial(this, Serial.list()[1], 115200);
  myPort.bufferUntil('\n');  // Read data until new line is encountered
}

void draw() {
  background(0);
  lights();  // Add basic lighting
  
  // Use accelerometer and gyroscope data to control the 3D object (sphere)
  pushMatrix();
  translate(width/2, height/2, 0);  // Move the origin to the center
  
  // Use accelerometer data for basic orientation
  float ax = sensorData[0];  // Accelerometer X-axis
  float ay = sensorData[1];  // Accelerometer Y-axis
  float az = sensorData[2];  // Accelerometer Z-axis
  
  // Rotate the cuboid based on accelerometer readings
  float tiltX = map(ax, -1, 1, -PI/4, PI/4);
  float tiltY = map(ay, -1, 1, -PI/4, PI/4);
  float tiltZ = map(az, -1, 1, -PI/4, PI/4);
  
  //rotateX(tiltX);  // Rotate along X-axis based on accelerometer
  //rotateY(tiltY);  // Rotate along Y-axis based on accelerometer
  //rotateZ(tiltZ);
  
  // Gyroscope data will be used for continuous rotation
  float gx = sensorData[3];  // Gyroscope X-axis (angular velocity)
  float gy = sensorData[4];  // Gyroscope Y-axis
  float gz = sensorData[5];  // Gyroscope Z-axis
  
  // Use the gyroscope data for rotation over time (constant angular speed)
  //rotateX(tiltX + gx * 0.05);
  //rotateY(tiltY + gy * 0.05);
  //rotateZ(tiltZ + gz * 0.05);

  // Magnetometer data: Magnetic field strength along X, Y, Z axes
  float mx = sensorData[6];  // Magnetometer X-axis
  float my = sensorData[7];  // Magnetometer Y-axis
  float mz = sensorData[8];  // Magnetometer Z-axis

  // Calculate heading (angle from magnetometer data)
  float heading = atan2(my, mx);  // Magnetometer heading, angle in radians
  
  // Adjust the overall orientation based on the heading from the magnetometer
  // This helps in stabilizing the orientation and correcting drift over time
  //rotateZ(heading);  // Apply the heading rotation based on the magnetometer

  rotateX(tiltX + gx * 0.05);
  rotateY(tiltY + gy * 0.05);
  rotateZ(tiltZ + gz * 0.05 + heading);
  
  
  fill(255, 0, 0);  // Red color
  noStroke();
  box(150, 100, 50);
  popMatrix();
}

// This function is called when new data arrives
void serialEvent(Serial myPort) {
  incomingData = myPort.readStringUntil('\n');  // Read the incoming serial data
  incomingData = trim(incomingData);  // Remove any whitespace
  
  // Split the comma-separated values into an array
  String[] tokens = split(incomingData, ',');
  
  // Parse the values into floats and store them in the sensorData array
  if (tokens.length == 9) {
    for (int i = 0; i < 9; i++) {
      sensorData[i] = float(tokens[i]);
    }
  }
}
