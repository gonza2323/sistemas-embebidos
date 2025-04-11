from flask import Flask
from flask import url_for
from flask import render_template
from flask import request
from flask import jsonify
from flask_socketio import SocketIO, emit
import serial
import time
import sys


# inicialización
app = Flask(__name__)
socketio = SocketIO(app, cors_allowed_origins="*")
port = '/dev/arduino' if not app.debug else 'rfc2217://localhost:4000'

try:
    if (app.debug):
        ser = serial.serial_for_url(
            port, baudrate=9600, bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_ONE, timeout=1,
            xonxoff=False, rtscts=False, dsrdtr=False, inter_byte_timeout=None
        )
    else:
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
                timestamp = current_time_ms = time.time_ns() // 1_000_000
                illumination = int.from_bytes(ser.read(2), byteorder='little')
                data = {}
                data["timestamp"] = timestamp
                data["illumination"] = illumination / 1024 * 100
                socketio.emit('new_data_point', data)
            socketio.sleep(0.10)
        
        except Exception as e:
            print(f"Error reading serial port: {e}")
            socketio.sleep(1)


@app.route('/', methods=['GET'])
def index():
    return render_template('index.html')


socketio.start_background_task(serial_read)
