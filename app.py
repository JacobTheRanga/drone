import os
from passlib.hash import pbkdf2_sha256
from flask import Flask, render_template, request, session, redirect, url_for
from serial import Serial
app = Flask(__name__)
hashedpass = '$pbkdf2-sha256$29000$GqP0HkMIIQRgDKHU2jsHgA$48Cmw2qfq2bsOehmXkWXWy1Ccx./V9dWbJmtUxOXAFg'
app.secret_key = os.urandom(64)

port = Serial('COM5')

@app.route('/login', methods = ['get', 'post'])
def login():
    if request.method != 'POST':
        return render_template('/login.html')
    password = request.form['pass']
    if pbkdf2_sha256.verify(password, hashedpass):
        session['user'] = 'admin'
        return redirect(url_for('home'))
    return render_template('/login.html', output ='Incorrect Password')

@app.route('/')
def home():
    if not session or session['user'] != 'admin':
        return redirect(url_for('login'))
    while 1:
        angle = {}
        rawAngle = port.readline().decode('utf-8')
        print(rawAngle)
        angle['x'] = rawAngle.split()[1]
        angle['y'] = rawAngle.split()[2]
        angle['z'] = rawAngle.split()[3]
        if request.method != 'POST':
            return render_template('/home.html', angle = angle)

@app.route('/angle')
def angle():
    return render_template('home.html')

if __name__ == '__main__':
    app.run(host='localhost', port=80)