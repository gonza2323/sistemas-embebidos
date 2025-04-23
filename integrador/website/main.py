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

ILLUMINATION_MSG = 'L'
BUTTON_MSG = 'B'

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
                msg_type = ser.read(1).decode()
                data = {}
                data["timestamp"] = timestamp

                if msg_type == ILLUMINATION_MSG:
                    illumination = int.from_bytes(ser.read(2), byteorder='little')
                    data["illumination"] = illumination / 1024 * 100
                    socketio.emit('illumination_update', data)
                elif msg_type == BUTTON_MSG:
                    button = int.from_bytes(ser.read(1))
                    state = bool.from_bytes(ser.read(1))
                    data["button"] = button
                    data["buttonState"] = state
                    socketio.emit('buttons_update', data)

            socketio.sleep(0.050)
        
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
