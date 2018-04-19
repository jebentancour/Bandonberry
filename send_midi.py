import RPi.GPIO as GPIO
import rtmidi_python as rtmidi
import time

PIN = 23

MIDI_PORT_NAME = "FLUID"
NOTE = 60
VELOCITY = 127
DURATION = 2

GPIO.setmode(GPIO.BCM)
GPIO.setup(PIN, GPIO.OUT)
GPIO.output(PIN, GPIO.LOW)

midi_out = rtmidi.MidiOut()

for port_name in midi_out.ports:
    if MIDI_PORT_NAME in port_name:
        midi_out.open_port(port_name)
        #GPIO.output(PIN, GPIO.HIGH)
        #GPIO.output(PIN, GPIO.LOW)
        GPIO.output(PIN, GPIO.HIGH)
        midi_out.send_message([0x90, NOTE, VELOCITY])
        GPIO.output(PIN, GPIO.LOW)
        time.sleep(DURATION)
        midi_out.send_message([0x80, NOTE, VELOCITY])

GPIO.cleanup()
