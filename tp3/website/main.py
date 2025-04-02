from flask import Flask
from flask import url_for
from flask import render_template
from flask import request
from flask import jsonify
from flask_socketio import SocketIO, emit
import serial
import sys

# valores iniciales
readingLuminosity = True
alarmTriggered = False


# tipos de mensajes
READ_ON = ord('Y');
READ_OFF = ord('N');
ALARM_TRIGGERED = ord('A');


# inicialización
app = Flask(__name__)
socketio = SocketIO(app, cors_allowed_origins="*")
port = '/dev/ttyUSB0' if not app.debug else 'rfc2217://localhost:4000'

try:
    ser = serial.serial_for_url(
        port, baudrate=9600, bytesize=serial.EIGHTBITS,
        parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_ONE, timeout=1,
        xonxoff=False, rtscts=False, dsrdtr=False, inter_byte_timeout=None
    )
except Exception as e:
    print(f"Error connecting to serial port '{port}'.")
    print("If --debug flag is set, make sure the simulator is running, otherwise, an Arduino board should be connected")
    sys.exit(1);


# lectura del puerto serial
def serial_read():
    while True:
        try:
            if ser.in_waiting > 0:
                data = ser.read(2)
                first_byte = data[0]
                second_byte = data[1]

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
                    illumination = int.from_bytes(data, byteorder='big')
                    socketio.emit('illumination_update', {'illumination': illumination})
            
            socketio.sleep(0.250)
        
        except Exception as e:
            print(f"Error reading serial port: {e}")
            socketio.sleep(1)


def convertNumStr2Byte(brightness):
    return max(0, min(int(brightness), 255))


@app.route('/update', methods=['POST'])
def update():
    try:
        data = request.get_json()
        readIllumination = data.get('readIllumination', None)

        if readIllumination is not None:
            char = READ_ON if readIllumination else READ_OFF
            ser.write(bytes([char]))

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


socketio.start_background_task(serial_read)
