#!/usr/bin/python

from BDN_MCP23S17 import MCP23S17
import rtmidi_python as rtmidi
import time
import sys

MIDI_PORT_NAME = "FLUID"
NOTE = 60
VELOCITY = 127
NOTE_ON = 0x90
NOTE_OFF = 0x80

try:
    midi_out = rtmidi.MidiOut()
    port_found = False
    for port_name in midi_out.ports:
        if MIDI_PORT_NAME in port_name:
            midi_out.open_port(port_name)
            print "MIDI port found!"
            port_found = True
    if not port_found:
        print "MIDI port not found, exit."
        sys.exit()

    mcp1 = MCP23S17(ce=1)
    mcp1.open()

    for x in range(0, 8):
        #print 'IN > {0}'.format(x)
        mcp1.setDirection(x, mcp1.DIR_INPUT)
        mcp1.setPullupMode(x, mcp1.PULLUP_ENABLED)

    for x in range(8, 16):
        #print 'OUT > {0}'.format(x)
        mcp1.setDirection(x, mcp1.DIR_OUTPUT)
        mcp1.digitalWrite(x, MCP23S17.LEVEL_HIGH)

    time.sleep(1)

    #old_btn = 1
    #
    #print "Starting reading pin A0 (CTRL+C to quit)"
    #while (True):
    #    new_btn = mcp1.digitalRead(12)
    #    if new_btn != old_btn:
    #        if new_btn == 0:
    #            #print "Button pressed!"
    #            midi_out.send_message([NOTE_ON, NOTE, VELOCITY])
    #        else:
    #            #print "Button released!"
    #            midi_out.send_message([NOTE_OFF, NOTE, VELOCITY])
    #
    #    old_btn = new_btn

    while (True):
        for x in range(8, 14):
            if x == 8:
                mcp1.digitalWrite(13, MCP23S17.LEVEL_HIGH)
            else:
                mcp1.digitalWrite(x-1, MCP23S17.LEVEL_HIGH)

            mcp1.digitalWrite(x, MCP23S17.LEVEL_LOW)

            data = mcp1.readGPIO()
            print '{0:b}'.format(data)

        print 'END'
        time.sleep(0.2)

finally:
    mcp1.close()