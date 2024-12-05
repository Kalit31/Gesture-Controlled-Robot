# Gesture-Controlled-Robot

## Step1: Network setup
Host the Raspaberry-Pi and the ESP32 on the same Wifi network.

## Step2: IMU and ESP32 module
Load the `send_imu_data` Arduino programs with the correct Wifi SSID and password for communicating with the RasPi.


## Step3: Raspaberry-Pi connected to the Sphero RVR
Run the python script to start API server, The RasPi receives roll, pitch and yaw data from the IMU, computes the orientation, and sends commands to the Sphero for appropiate motion.
```bash
python3 server.py
```
The Sphero RVR moves based on the gestures, Hooray!