from flask import Flask
from flask import render_template
from flask import request
from flask import jsonify
from flask_socketio import SocketIO
from common.arduino import SerialConnection
import os


# variables
led09brightness = 255
led10brightness = 64
led11brightness = 16
led13status = True


# setup app
debug_serial = os.getenv('FLASK_DEBUG') == '1' or os.getenv('DEBUG_SERIAL') == '1'
app = Flask(__name__)
socketio = SocketIO(app, cors_allowed_origins="*")
arduino = SerialConnection(verbose=debug_serial)


def serial_read():
    while True:
        try:
            if arduino.in_waiting() > 0:
                with arduino:
                    analogValue = int.from_bytes(arduino.read(2), byteorder='little')
                illumination = int(analogValue / 1024 * 100)
                socketio.emit('illumination_update', {'illumination': illumination})
                
            socketio.sleep(0.250)
        
        except Exception as e:
            print(e)
            socketio.sleep(1)


def convertNumStr2Byte(brightness):
    return max(0, min(int(brightness), 255))


@app.route('/update', methods=['POST'])
def update():
    try:
        data = request.get_json()

        led09 = convertNumStr2Byte(data.get('led09', 0))
        led10 = convertNumStr2Byte(data.get('led10', 0))
        led11 = convertNumStr2Byte(data.get('led11', 0))
        led13 = data.get('led13', '') == 'on'

        # envío de datos al arduino
        data = bytes([
            led09,
            led10,
            led11,
            int(led13)
        ])
        
        with arduino:
            arduino.write(data)
            
        # actualizamos el estado en el servidor
        global led09brightness, led10brightness, led11brightness, led13status
        led09brightness = led09
        led10brightness = led10
        led11brightness = led11
        led13status = led13

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


# inicializar app
socketio.start_background_task(serial_read)

if __name__ == "__main__":
    flask_debug = os.getenv('FLASK_DEBUG') == '1'
    socketio.run(app, debug=flask_debug)
