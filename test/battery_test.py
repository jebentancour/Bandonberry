#!/usr/bin/python

import time
import rtmidi_python as rtmidi
import RPi.GPIO as GPIO


NOTE_ON = 0x90
NOTE_OFF = 0x80

MIDI_PORT_NAME = "FLUID"
VELOCITY = 96
NOTE1 = 60
NOTE2 = 68

midi_out = rtmidi.MidiOut()
for port_name in midi_out.ports:
    if MIDI_PORT_NAME in port_name:
        midi_out.open_port(port_name)

PIN = 4

GPIO.setmode(GPIO.BCM)
GPIO.setup(PIN, GPIO.OUT)

servo = GPIO.PWM(PIN,50)
servo.start(0)

midi_out.send_message([NOTE_ON, NOTE1, VELOCITY])  # Toco notas
midi_out.send_message([NOTE_ON, NOTE2, VELOCITY])

try:
    # Muevo el motor en un loop infinito
    while True:
            servo.ChangeDutyCycle(10)                     # Servo abierto
            time.sleep(0.5)
            servo.ChangeDutyCycle(0)                      # Servo quieto
            time.sleep(0.5)
            servo.ChangeDutyCycle(5)                      # Cerrado
            time.sleep(0.5)
            servo.ChangeDutyCycle(0)
            time.sleep(0.5)

finally:
    midi_out.send_message([NOTE_OFF, NOTE1, 0x00])  # Apago notas
    midi_out.send_message([NOTE_OFF, NOTE2, 0x00])
    servo.stop()
    GPIO.cleanup()
