#!/usr/bin/python

import rtmidi_python as rtmidi
import RPi.GPIO as GPIO
import time

NOTE_ON = 0x90
NOTE_OFF = 0x80

MIDI_PORT_NAME = "FLUID"
VELOCITY = 127
NOTE = 60

midi_out = rtmidi.MidiOut()
for port_name in midi_out.ports:
    if MIDI_PORT_NAME in port_name:
        midi_out.open_port(port_name)

PIN = 4

GPIO.setmode(GPIO.BCM)
GPIO.setup(PIN, GPIO.OUT)

servo = GPIO.PWM(PIN,50)
servo.start(7.5)                                          # generate PWM signal with 7.5% duty cycle

try:
    while True:                                           # execute loop forever
        servo.ChangeDutyCycle(5)
        time.sleep(0.5)                                     # sleep for 1 second
        servo.ChangeDutyCycle(0)
        time.sleep(0.5)                                     # sleep for 1 second
        servo.ChangeDutyCycle(10)
        time.sleep(0.5)                                     # sleep for 1 second
        servo.ChangeDutyCycle(0)
        midi_out.send_message([NOTE_ON, NOTE, VELOCITY])
        time.sleep(2)                                       # sleep for 1 second
        midi_out.send_message([NOTE_OFF, NOTE, VELOCITY])
        time.sleep(0.5)
except KeyboardInterrupt:
    servo.stop()
    GPIO.cleanup()
