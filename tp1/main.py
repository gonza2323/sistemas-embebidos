from flask import Flask
from flask import url_for
from flask import render_template
from flask import request
from flask import jsonify
from livereload import Server


app = Flask(__name__)
app.debug = True

@app.route('/', methods=['GET'])
def index():
    return render_template('index.html')


@app.route('/update', methods=['POST'])
def update():
    try:
        data = request.get_json()
        
        print("Received data:", data)

        return jsonify(data), 200

    except Exception as e:
        return jsonify({"error": "Invalid JSON or bad request", "message": str(e)}), 400


server = Server(app.wsgi_app)
server.watch("templates/*.*") # or what-have-you
server.watch("static/*.*") # or what-have-you
server.serve(port=5000)
