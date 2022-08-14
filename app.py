import os
from passlib.hash import pbkdf2_sha256
from flask import Flask, render_template, request, session, redirect, url_for
app = Flask(__name__)
hashedpass = '$pbkdf2-sha256$29000$GqP0HkMIIQRgDKHU2jsHgA$48Cmw2qfq2bsOehmXkWXWy1Ccx./V9dWbJmtUxOXAFg'
app.secret_key = os.urandom(64)

import drone

@app.route('/login', methods = ['get', 'post'])
def login():
    if request.method != 'POST':
        return render_template('/login.html')
    password = request.form['pass']
    if pbkdf2_sha256.verify(password, hashedpass):
        session['user'] = 'admin'
        return redirect(url_for('home'))
    return render_template('/login.html', output ='Incorrect Password')

@app.route('/', methods=['get', 'post'])
def home():
    if not session or session['user'] != 'admin':
        return redirect(url_for('login'))
    if request.method != 'POST':
        return render_template('/home.html')
    drone.blink()

if __name__ == '__main__':
    app.run(host='localhost', port=80)