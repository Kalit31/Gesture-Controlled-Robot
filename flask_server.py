from flask import Flask, request, jsonify
import json
import os
import sys
import time
import asyncio

MAX_SPEED = 100
INITIAL_NUMBER_OF_REQUESTS_TO_IGNORE = 30
NUMBER_OF_REQUESTS_TO_CALIBRATE = INITIAL_NUMBER_OF_REQUESTS_TO_IGNORE + 10
sys.path.append('/home/kinani/Desktop/sphero-sdk-raspberrypi-python')
from sphero_sdk import SpheroRvrObserver
rvr = SpheroRvrObserver()

count = 0
calibrated_yaw_sum = 0.0
calibrated_yaw = 0.0

app = Flask(__name__)

@app.route('/')
def home():
    return "Hello from Raspberry Pi!"

@app.route('/data', methods=['POST'])
def receive_data():
    global count
    global calibrated_yaw_sum
    global calibrated_yaw
    data = request.get_json()
    count += 1
    # data_dict = json.load(data)
    print("Received data:", data)
    if count < INITIAL_NUMBER_OF_REQUESTS_TO_IGNORE:
        return '', 204  # 204 No Content
    elif count < NUMBER_OF_REQUESTS_TO_CALIBRATE:
        calibrated_yaw_sum += data["yaw"]
        return '', 204  # 204 No Content

    if count == NUMBER_OF_REQUESTS_TO_CALIBRATE:
        calibrated_yaw = calibrated_yaw_sum/(NUMBER_OF_REQUESTS_TO_CALIBRATE-INITIAL_NUMBER_OF_REQUESTS_TO_IGNORE)
        print(f"Calibrated Yaw value: {calibrated_yaw}")
        return '', 204  # 204 No Content

    move(data)
    # return jsonify({"status": "success", "received": data})
    return '', 204  # 204 No Content

def move(data):
    global calibrated_yaw
    drive_fwd = 0
    speed = 0
    heading_value = float(data["yaw"])
    pitch_value = float(data["pitch"])
    print(f"Got pitch: {pitch_value}")
    if pitch_value > 0:
        drive_fwd = 1
    abs_pitch = abs(pitch_value)

    if (abs_pitch <= 25):
        # speed = 0
        return

    if (abs_pitch >= 70 and abs_pitch <=90):
        speed = MAX_SPEED
    else:
        speed = int(((abs_pitch-25)/45)*(MAX_SPEED-1))

    heading_value_norm = int((heading_value - calibrated_yaw)%360)

    # if drive_fwd:
    #     drive_forward(speed, heading)
    # else:
    #     drive_backward(speed, heading)
    print(f"Driving with speed:{speed}, flags:{drive_fwd}")
    rvr.drive_with_heading(
        speed=speed,       # This is out of 255, where 255 corresponds to 2 m/s
        heading=heading_value_norm,      # Valid heading values are 0-359
        # time_to_drive=time # Driving duration in seconds
        flags=drive_fwd
    )


def drive_forward(speed, heading, time=0.3):
    print(f"Driving forward with speed {speed}")
    rvr.drive_control.drive_forward_seconds(
        speed=speed,       # This is out of 255, where 255 corresponds to 2 m/s
        heading=heading,      # Valid heading values are 0-359
        time_to_drive=time # Driving duration in seconds
    )
    print("Driving forward done")

def drive_backward(speed, heading, time=0.3):
    print(f"Driving backward with speed {speed}")
    rvr.drive_control.drive_backward_seconds(
        speed=speed,       # This is out of 255, where 255 corresponds to 2 m/s
        heading=heading,      # Valid heading values are 0-359
        time_to_drive=time # Driving duration in seconds
    )
    print("Driving backward done")


def sphero_setup():
    # Make sure RVR is awake and ready to receive commands
    rvr.wake()
    # Wait for RVR to wake up
    time.sleep(2)

    # Resetting our heading makes the current heading 0
    rvr.drive_control.reset_heading()
    print("Setup done")

if __name__ == '__main__':
    sphero_setup()
    app.run(host='0.0.0.0', port=5000)