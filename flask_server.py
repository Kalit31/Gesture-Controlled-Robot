from flask import Flask, request, jsonify
import json
import os
import sys
import time
import asyncio

MAX_SPEED = 140
INITIAL_NUMBER_OF_REQUESTS_TO_IGNORE = 30
NUMBER_OF_REQUESTS_TO_CALIBRATE = INITIAL_NUMBER_OF_REQUESTS_TO_IGNORE + 10
CONSTANT_ROTATION_SPEED = 3
sys.path.append('/home/kinani/Desktop/sphero-sdk-raspberrypi-python')
from sphero_sdk import SpheroRvrObserver
rvr = SpheroRvrObserver()

count = 0
calibrated_yaw_sum = 0.0
calibrated_yaw = 0.0
previous_heading = 0

app = Flask(__name__)

@app.route('/')
def home():
    return "Hello from Raspberry Pi!"

@app.route('/data', methods=['POST'])
def receive_data():
    global count
    global calibrated_yaw_sum
    global calibrated_yaw
    global previous_heading
    data = request.get_json()
    count += 1
    # data_dict = json.load(data)
    print("Received data:", data)
    if count < INITIAL_NUMBER_OF_REQUESTS_TO_IGNORE:
        return '', 204  # 204 No Content
    # elif count < NUMBER_OF_REQUESTS_TO_CALIBRATE:
    #     calibrated_yaw_sum += data["yaw"]
    #     return '', 204  # 204 No Content

    if count == INITIAL_NUMBER_OF_REQUESTS_TO_IGNORE:
        # calibrated_yaw = calibrated_yaw_sum/(NUMBER_OF_REQUESTS_TO_CALIBRATE-INITIAL_NUMBER_OF_REQUESTS_TO_IGNORE)
        previous_heading = 0
        # print(f"Calibrated Yaw value: {calibrated_yaw}")
        return '', 204  # 204 No Content

    move(data)
    # return jsonify({"status": "success", "received": data})
    return '', 204  # 204 No Content

def calculate_speed(pitch):
    drive_fwd = 0
    if pitch > 0:
        drive_fwd = 1

    abs_pitch = abs(pitch)

    if (abs_pitch <= 25):
        speed = 0
        return (speed, drive_fwd)

    if (abs_pitch >= 70 and abs_pitch <=90):
        speed = MAX_SPEED
    else:
        speed = int(((abs_pitch-25)/45)*(MAX_SPEED-1))

    return (speed, drive_fwd)

def calculate_heading(yaw):
    global calibrated_yaw
    # global previous_heading
    difference_in_heading = yaw - calibrated_yaw
    intermidate_value = (difference_in_heading)%360
    heading_value_norm = int((-1 * intermidate_value) % 360)

    if (heading_value_norm > 335 or heading_value_norm < 25):
        heading_value_norm = 0
    else:
        if heading_value_norm > 180:
            heading_value_norm = 340
        elif heading_value_norm < 180:
            heading_value_norm = 20

    return heading_value_norm

def calculate_heading_using_roll(roll):
    heading_value_norm = int((360-roll)%360)

    if (heading_value_norm > 320 or heading_value_norm < 40):
        heading_value_norm = 0
    else:
        if heading_value_norm > 180:
            heading_value_norm = 340
        elif heading_value_norm < 180:
            heading_value_norm = 20
    return heading_value_norm

def move(data):
    global previous_heading
    yaw_value = float(data["yaw"])
    pitch_value = float(data["pitch"])
    roll_value = float(data["roll"])
    print(f"Got pitch: {pitch_value}")
    (speed, drive_fwd) = calculate_speed(pitch_value)
    # heading = calculate_heading(yaw_value)
    heading = calculate_heading_using_roll(roll_value)

    if (speed == 0 and heading > 0):
        heading = (previous_heading + heading)%360
        previous_heading = heading
        speed = CONSTANT_ROTATION_SPEED
    else:
        heading = previous_heading

    print(f"Driving with speed:{speed}, flags:{drive_fwd}, headin:{heading}")
    rvr.drive_with_heading(
        speed=speed,       # This is out of 255, where 255 corresponds to 2 m/s
        heading=heading,      # Valid heading values are 0-359
        # time_to_drive=time # Driving duration in seconds
        flags=drive_fwd
    )

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