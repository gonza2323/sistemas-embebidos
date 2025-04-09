from flask import Flask
from flask import url_for
from flask import render_template
from flask import request
from flask import jsonify
from flask_socketio import SocketIO, emit
import serial
import sys

# valores iniciales
led09brightness = 255
led10brightness = 64
led11brightness = 16
led13status = True


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


def serial_read():
    while True:
        try:
            if ser.in_waiting > 0:
                illumination = int.from_bytes(ser.read(2), byteorder='big')
                socketio.emit('illumination_update', {'illumination': illumination})
            
            socketio.sleep(0.250)
        
        except Exception as e:
            print(f"Error reading serial port: {e}")
            socketio.sleep(1)


def convertNumStr2Byte(brightness):
    return max(0, min(int(brightness), 255))


def updateArduino():
    data = bytes([
        led09brightness,
        led10brightness,
        led11brightness,
        int(led13status)
    ])
    ser.write(data)


@app.route('/update', methods=['POST'])
def update():
    try:
        data = request.get_json()

        global led09brightness, led10brightness, led11brightness, led13status
        led09brightness = convertNumStr2Byte(data.get('led09', 0))
        led10brightness = convertNumStr2Byte(data.get('led10', 0))
        led11brightness = convertNumStr2Byte(data.get('led11', 0))
        led13status = data.get('led13', '') == 'on'

        updateArduino()

        return '', 204

    except Exception as e:
        return jsonify({"error": "Invalid JSON or bad request", "message": str(e)}), 400


@app.route('/', methods=['GET'])
def index():
    return render_template(
        'index.html',
        led09brightness = led09brightness,
        led10brightness = led10brightness,
        led11brightness = led11brightness,
        led13status = led13status
    )


socketio.start_background_task(serial_read)
updateArduino();
