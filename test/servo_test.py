#!/usr/bin/python

import RPi.GPIO as GPIO
import time

PIN = 4

GPIO.setmode(GPIO.BCM)
GPIO.setup(PIN, GPIO.OUT)

servo = GPIO.PWM(PIN,50)
servo.start(7.5)                             # generate PWM signal with 7.5% duty cycle

try:
    while True:                                                       # execute loop forever
        servo.ChangeDutyCycle(4.5)
        time.sleep(1)                                      # sleep for 1 second
        servo.ChangeDutyCycle(8.5)
        time.sleep(1)                                     # sleep for 1 second
        servo.ChangeDutyCycle(11.5)
        time.sleep(1)                                     # sleep for 1 second
except KeyboardInterrupt:
    servo.stop()
    GPIO.cleanup()
