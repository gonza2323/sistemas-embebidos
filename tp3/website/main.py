from flask import Flask
from flask import render_template
from flask import request
from flask import jsonify
from flask_socketio import SocketIO
from common.arduino import SerialConnection
from datetime import datetime
import struct
import time
import os


# tipos de mensajes
ERASE_MEMORY_MSG = b'M'
REQUEST_EVENTS_MSG = b'R'
SYNC_MSG = b'S'
SINGLE_EVENT_MSG = 255


# setup app
debug_serial = os.getenv('FLASK_DEBUG') == '1' or os.getenv('DEBUG_SERIAL') == '1'
app = Flask(__name__)
socketio = SocketIO(app, cors_allowed_origins="*")
arduino = SerialConnection(verbose=debug_serial)


# lectura del puerto serial
def serial_read():
    while True:
        try:
            if arduino.in_waiting() > 0:
                with arduino:
                    data = arduino.read(1)
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
            print(e)
            socketio.sleep(1)


def readEvent():
    with arduino:
        button = int.from_bytes(arduino.read())
        unixTimestampSeconds = int.from_bytes(arduino.read(4), byteorder='little')
        unixTimestampMillis = int.from_bytes(arduino.read(2), byteorder='little')
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

    try:
        with arduino:
            arduino.write(SYNC_MSG)
            arduino.write(data)
    except Exception as e:
        print(e)


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
        with arduino:
            arduino.write(REQUEST_EVENTS_MSG)
        return '', 204

    except Exception as e:
        return jsonify({"error": "Invalid JSON or bad request", "message": str(e)}), 400


@app.route('/erase-memory', methods=['POST'])
def erase_memory_route():
    try:
        with arduino:
            arduino.write(ERASE_MEMORY_MSG)
        return '', 204

    except Exception as e:
        return jsonify({"error": "", "message": str(e)}), 400


@app.route('/', methods=['GET'])
def index():
    return render_template('index.html')


# inicializar app
sync_time()
socketio.start_background_task(serial_read)

if __name__ == "__main__":
    flask_debug = os.getenv('FLASK_DEBUG') == '1'
    socketio.run(app, debug=flask_debug)
