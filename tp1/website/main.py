from flask import Flask
from flask import url_for
from flask import render_template
from flask import request
from flask import jsonify
from flask_socketio import SocketIO, emit
import serial


app = Flask(__name__)
socketio = SocketIO(app, cors_allowed_origins="*")

ser = serial.serial_for_url(
    'rfc2217://localhost:4000', baudrate=9600, bytesize=serial.EIGHTBITS,
    parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_ONE, timeout=1,
    xonxoff=False, rtscts=False, dsrdtr=False, inter_byte_timeout=None
)


def serial_read():
    while True:
        try:
            if ser.in_waiting > 0:
                msgType = ser.read(1);
                illumination = int.from_bytes(ser.read(2), byteorder='big')
                
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

        led09brightness = convertNumStr2Byte(data.get('led09', 0))
        led10brightness = convertNumStr2Byte(data.get('led10', 0))
        led11brightness = convertNumStr2Byte(data.get('led11', 0))
        led13status = 1 if data.get('led13', '') == 'on' else 0

        serialData = bytes([
            led09brightness,
            led10brightness,
            led11brightness,
            led13status,
            ord('\n')
        ])
        
        ser.write(serialData)

        return '', 204

    except Exception as e:
        return jsonify({"error": "Invalid JSON or bad request", "message": str(e)}), 400


@app.route('/', methods=['GET'])
def index():
    return render_template('index.html')


socketio.start_background_task(serial_read)
