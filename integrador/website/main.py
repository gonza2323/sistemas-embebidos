from flask import Flask
from flask import render_template
from flask import request
from flask import jsonify
from flask_socketio import SocketIO
from common.arduino import SerialConnection
import struct
import time
import os


# tipos de mensaje
ILLUMINATION_MSG = 'L'
BUTTON_MSG = 'B'


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
                timestamp = time.time_ns() // 1_000_000
                with arduino:
                    msg_type = arduino.read(1).decode()
                data = {}
                data["timestamp"] = timestamp

                if msg_type == ILLUMINATION_MSG:
                    with arduino:
                        illumination = int.from_bytes(arduino.read(2), byteorder='little')
                    data["illumination"] = illumination / 1024 * 100
                    socketio.emit('illumination_update', data)
                elif msg_type == BUTTON_MSG:
                    with arduino:
                        button = int.from_bytes(arduino.read(1))
                        state = bool.from_bytes(arduino.read(1))
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
    try:
        with arduino:
            arduino.write(data)
    except Exception as e:
        print(e)


@app.route('/', methods=['GET'])
def index():
    return render_template('index.html')


# inicializar app
socketio.start_background_task(serial_read)

if __name__ == "__main__":
    flask_debug = os.getenv('FLASK_DEBUG') == '1'
    socketio.run(app, debug=flask_debug)
