from flask import Flask
from flask import url_for
from flask import render_template
from flask import request


app = Flask(__name__)

@app.route('/', methods=['GET'])
def index():
 ##Código Python##
 return render_template('index.html')
