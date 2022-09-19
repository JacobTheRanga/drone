from pyfirmata import Arduino, util
from time import sleep

board = Arduino('COM4')
pin13 = board.get_pin('d:13:o')

def blink():
    while True:
        pin13.write(1)
        sleep(1)
        pin13.write(0)
        sleep(1)
