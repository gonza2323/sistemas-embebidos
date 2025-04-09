from flask import Flask
from flask import url_for
from flask import render_template
from flask import request
from flask import jsonify
from flask_socketio import SocketIO, emit
from datetime import datetime
import serial
import struct
import time
import sys

# tipos de mensajes
ERASE_MEMORY_MSG = b'M'
REQUEST_EVENTS_MSG = b'R'
SYNC_MSG = b'S'
SINGLE_EVENT_MSG = 255


# inicialización
app = Flask(__name__)
socketio = SocketIO(app, cors_allowed_origins="*")
port = '/dev/ttyACM0' if not app.debug else 'rfc2217://localhost:4000'

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
                data = ser.read(1)
                first_byte = data[0]

                if (first_byte == SINGLE_EVENT_MSG):
                    socketio.emit('single_event', readEvent())
                else:
                    eventAmount = first_byte
                    events = []
                    for i in range(eventAmount):
                        events.append(readEvent())
                    socketio.emit('all_events', events)
            
            socketio.sleep(0.100)
        
        except Exception as e:
            print(f"Error reading serial port: {e}")
            socketio.sleep(1)


def readEvent():
    button = int.from_bytes(ser.read())
    unixTimestampSeconds = int.from_bytes(ser.read(4), byteorder='little')
    unixTimestampMillis = int.from_bytes(ser.read(2), byteorder='little')
    unixTimestamp = unixTimestampSeconds + unixTimestampMillis / 1000.0
        
    event = {}
    event["event"] = "Botón " + str(button)
    event["timestamp"] = unixTimestamp
    event["time"] = timestamp_to_local_datetime(unixTimestamp)
    return event


def timestamp_to_local_datetime(timestamp):
    dt = datetime.fromtimestamp(timestamp)
    milliseconds = int((timestamp % 1) * 1000)
    return dt.strftime(f'%Y-%m-%d %H:%M:%S.{milliseconds:03d} %Z')


def sync_time():
    timeMs = time.time_ns() // 1_000_000
    data = struct.pack('<IH', timeMs // 1000, timeMs % 1000);
    print(timeMs);

    ser.write(SYNC_MSG)
    ser.write(data)


@app.route('/sync-time', methods=['POST'])
def sync_time_route():
    try:
        sync_time()
        return '', 204

    except Exception as e:
        return jsonify({"error": "Invalid JSON or bad request", "message": str(e)}), 400


@app.route('/request-events', methods=['GET'])
def request_events_route():
    try:
        ser.write(REQUEST_EVENTS_MSG)
        return '', 204

    except Exception as e:
        return jsonify({"error": "Invalid JSON or bad request", "message": str(e)}), 400


@app.route('/erase-memory', methods=['POST'])
def erase_memory_route():
    try:
        ser.write(ERASE_MEMORY_MSG)
        return '', 204

    except Exception as e:
        return jsonify({"error": "Invalid JSON or bad request", "message": str(e)}), 400


@app.route('/', methods=['GET'])
def index():
    return render_template('index.html')


sync_time()
socketio.start_background_task(serial_read)
