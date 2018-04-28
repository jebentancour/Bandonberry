#!/usr/bin/python

from BDN_MCP23S17 import MCP23S17
import RPi.GPIO as GPIO
import rtmidi_python as rtmidi
import time
import sys

MIDI_PORT_NAME = "FLUID"
NOTE = 60
VELOCITY = 127
NOTE_ON = 0x90
NOTE_OFF = 0x80

PIN = 23
GPIO.setmode(GPIO.BCM)
GPIO.setup(PIN, GPIO.OUT)
GPIO.output(PIN, GPIO.LOW)

try:
    midi_out = rtmidi.MidiOut()
    port_found = False
    for port_name in midi_out.ports:
        if MIDI_PORT_NAME in port_name:
            midi_out.open_port(port_name)
            print "MIDI port foun!"
            port_found = True
    if not port_found:
        print "MIDI port not found, exit."
        sys.exit()

    mcp1 = MCP23S17(ce=1)
    mcp1.open()

    for x in range(0, 16):
        mcp1.setDirection(x, mcp1.DIR_INPUT)
        mcp1.setPullupMode(x, mcp1.PULLUP_ENABLED)

    old_btn = 1

    print "Starting reading pin A0 (CTRL+C to quit)"
    while (True):
        new_btn = mcp1.digitalRead(0)
        if new_btn != old_btn:
            if new_btn == 0:
                #print "Button pressed!"
                GPIO.output(PIN, GPIO.HIGH)
                midi_out.send_message([NOTE_ON, NOTE, VELOCITY])
                GPIO.output(PIN, GPIO.LOW)
            else:
                #print "Button released!"
                midi_out.send_message([NOTE_OFF, NOTE, VELOCITY])

        old_btn = new_btn

finally:
    mcp1.close()
