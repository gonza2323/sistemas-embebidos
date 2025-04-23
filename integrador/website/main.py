from flask import Flask
from flask import url_for
from flask import render_template
from flask import request
from flask import jsonify
from flask_socketio import SocketIO, emit
import serial
import struct
import time
import sys


# inicialización
app = Flask(__name__)
socketio = SocketIO(app, cors_allowed_origins="*")
port = '/dev/arduino'

try:
    ser = serial.Serial(
        port, baudrate=9600, bytesize=serial.EIGHTBITS,
        parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_ONE, timeout=1,
        xonxoff=False, rtscts=False, dsrdtr=False, inter_byte_timeout=None,
        exclusive=None)
except Exception as e:
    print(f"Error connecting to serial port '{port}'.")
    print("If --debug flag is set, make sure the simulator is running, otherwise, an Arduino board should be connected")
    sys.exit(1);


# lectura del puerto serial
def serial_read():
    while True:
        try:
            if ser.in_waiting > 0:
                timestamp = time.time_ns() // 1_000_000
                illumination = int.from_bytes(ser.read(2), byteorder='little')
                buttonA = bool.from_bytes(ser.read(1))
                buttonB = bool.from_bytes(ser.read(1))
                data = {}
                data["timestamp"] = timestamp
                data["illumination"] = illumination / 1024 * 100
                data["buttonA"] = buttonA
                data["buttonB"] = buttonB
                socketio.emit('new_data_point', data)
            socketio.sleep(0.10)
        
        except Exception as e:
            print(f"Error reading serial port: {e}")
            socketio.sleep(1)


@socketio.on('data')
def handle_data(data):
    data = struct.pack('<f', data)
    ser.write(data)


@app.route('/', methods=['GET'])
def index():
    return render_template('index.html')


socketio.start_background_task(serial_read)
