#!/usr/bin/python

import RPi.GPIO as GPIO
import time

PIN = 4

GPIO.setmode(GPIO.BCM)
GPIO.setup(PIN, GPIO.OUT)

servo = GPIO.PWM(PIN,50)
servo.start(0)

try:
    while True:                                           # execute loop forever
        servo.ChangeDutyCycle(5)                          # 0
        print('5')
        time.sleep(1)                                     # sleep for 1 second
        servo.ChangeDutyCycle(7.5)                        # 90
        print('7.5')
        time.sleep(1)                                     # sleep for 1 second
        servo.ChangeDutyCycle(10)                         # 180
        print('10')
        time.sleep(1)                                     # sleep for 1 second

except KeyboardInterrupt:
    servo.stop()
    GPIO.cleanup()
