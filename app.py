import os
from passlib.hash import pbkdf2_sha256
from flask import Flask, render_template, request, session, redirect, url_for
app = Flask(__name__)
hashedpass = '$pbkdf2-sha256$29000$d44xhjAmZIzx3rv3fs.Zcw$BB8iMBbPn42h7lOqDkxS5bqSpmRJTiimLZYAwfdM/ko'
app.secret_key = os.urandom(64)

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
    return render_template('/main.html')

if __name__ == '__main__':
    app.run(host='localhost', port=80)