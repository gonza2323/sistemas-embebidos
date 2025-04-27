from flask import Flask
from flask import render_template
from flask_socketio import SocketIO
from common.arduino import SerialConnection
import time
import os


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
                    illumination = int.from_bytes(arduino.read(2), byteorder='little')
                data = {}
                data["timestamp"] = timestamp
                data["illumination"] = illumination / 1024 * 100
                socketio.emit('new_data_point', data)
            
            socketio.sleep(0.10)
        
        except Exception as e:
            print(e)
            socketio.sleep(1)


@app.route('/', methods=['GET'])
def index():
    return render_template('index.html')


# inicializar app
socketio.start_background_task(serial_read)

if __name__ == "__main__":
    flask_debug = os.getenv('FLASK_DEBUG') == '1'
    socketio.run(app, debug=flask_debug)
