from flask import Flask
from flask import render_template
from flask import request
from flask import jsonify
from flask_socketio import SocketIO
from common.arduino import SerialConnection
import os

# valores iniciales
readingLuminosity = True
alarmTriggered = False


# tipos de mensajes
READ_ON = ord('Y');
READ_OFF = ord('N');
ALARM_TRIGGERED = ord('A');


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
                    data = arduino.read(2)
                first_byte = data[0]

                global readingLuminosity, alarmTriggered

                if (first_byte == READ_ON):
                    readingLuminosity = True
                    socketio.emit('read_on', 0)
                elif (first_byte == READ_OFF):
                    readingLuminosity = False
                    alarmTriggered = False
                    socketio.emit('read_off', 0)
                elif (first_byte == ALARM_TRIGGERED):
                    alarmTriggered = True
                    socketio.emit('alarm', 0)
                else:
                    illumination_data = int.from_bytes(data, byteorder='big')
                    illumination = int(illumination_data / 1024 * 100)
                    socketio.emit('illumination_update', {'illumination': illumination})
            
            socketio.sleep(0.250)
        
        except Exception as e:
            print(e)
            socketio.sleep(1)


@app.route('/update', methods=['POST'])
def update():
    try:
        data = request.get_json()
        readIllumination = data.get('readIllumination', None)

        if readIllumination is not None:
            char = READ_ON if readIllumination else READ_OFF
            with arduino:
                arduino.write(bytes([char]))

        return '', 204

    except Exception as e:
        return jsonify({"error": "Invalid JSON or bad request", "message": str(e)}), 400


@app.route('/', methods=['GET'])
def index():
    return render_template(
        'index.html',
        readingLuminosity = readingLuminosity,
        alarmTriggered = alarmTriggered
    )


# inicializar app
socketio.start_background_task(serial_read)

if __name__ == "__main__":
    flask_debug = os.getenv('FLASK_DEBUG') == '1'
    socketio.run(app, debug=flask_debug)
