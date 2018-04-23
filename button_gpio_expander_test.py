#!/usr/bin/python

from BDN_MCP23S17 import MCP23S17
import rtmidi_python as rtmidi
import time

MIDI_PORT_NAME = "FLUID"
NOTE = 60
VELOCITY = 127

midi_out = rtmidi.MidiOut()

for port_name in midi_out.ports:
    if MIDI_PORT_NAME in port_name:
        midi_out.open_port(port_name)

mcp1 = MCP23S17(ce=1)
mcp1.open()

for x in range(0, 16):
    mcp1.setDirection(x, mcp1.DIR_INPUT)
    mcp1.setPullupMode(x, mcp1.PULLUP_ENABLED)

old_btn = 1

print "Starting reading pins (CTRL+C to quit)"
while (True):
    #data = mcp1.readGPIO()
    #bin = "{0:b}".format(data) # binary
    #print bin
    #time.sleep(0.5)

    new_btn = mcp1.digitalRead(0)
    if new_btn != old_btn:
        if new_btn == 0:
            #print "Button pressed!"
            midi_out.send_message([0x90, NOTE, VELOCITY])
        else:
            #print "Button released!"
            midi_out.send_message([0x80, NOTE, VELOCITY])

    old_btn = new_btn
