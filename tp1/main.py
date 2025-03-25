from flask import Flask
from flask import url_for
from flask import render_template
from flask import request
from livereload import Server


app = Flask(__name__)
app.debug = True

@app.route('/', methods=['GET'])
def index():
 ##Código Python##
 return render_template('index.html')

server = Server(app.wsgi_app)
server.watch("templates/*.*") # or what-have-you
server.watch("static/*.*") # or what-have-you
server.serve(port=5000)
