#!/usr/bin/python

from BDN_MCP23S17 import MCP23S17
import rtmidi_python as rtmidi
import time
import sys

MIDI_PORT_NAME = "FLUID"
VELOCITY = 127
NOTE_ON = 0x90
NOTE_OFF = 0x80

# Creates a list containing 5 lists, each of 8 items, all set to 0
w, h = 8, 6;
right_notes_matrix = [[0 for x in range(w)] for y in range(h)]
#right_notes_matrix[4][3] = 59
#right_notes_matrix[4][4] = 65
#right_notes_matrix[5][3] = 43

prev_data = [65279, 65023, 64511, 63487, 61439, 57343]
new_data = [0 for x in range(6)]

try:
    midi_out = rtmidi.MidiOut()
    port_found = False
    for port_name in midi_out.ports:
        if MIDI_PORT_NAME in port_name:
            midi_out.open_port(port_name)
            print "MIDI port found."
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

    while (True):
        for x in range(8, 14):
            if x == 8:
                mcp1.digitalWrite(13, MCP23S17.LEVEL_HIGH)
            else:
                mcp1.digitalWrite(x-1, MCP23S17.LEVEL_HIGH)

            mcp1.digitalWrite(x, MCP23S17.LEVEL_LOW)

            new_data[x-8] = mcp1.readGPIO()

            if new_data[x-8] != prev_data[x-8]:
                for y in range(8):
                    new_bit = (new_data[x-8] >> y) & 0x01
                    prev_bit = (prev_data[x-8] >> y) & 0x01
                    if new_bit != prev_bit:
                        #print '{0}:{1:b} > {2} {3:b}'.format(x-8, new_data[x-8] & 0xFF, y, new_bit)
                        if new_bit:
                            #note = right_notes_matrix[x-8][y]
                            note = 40 + (x-8)*10 + y
                            midi_out.send_message([NOTE_OFF, note, VELOCITY])
                            print 'Note OFF [{0}][{1}] {2}'.format(x-8, y, note)
                        else:
                            #note = right_notes_matrix[x-8][y]
                            note = 40 + (x-8)*10 + y
                            midi_out.send_message([NOTE_ON, note, VELOCITY])
                            print 'Note ON [{0}][{1}] {2}'.format(x-8, y, note)

            prev_data[x-8] = new_data[x-8]

        #time.sleep(0.2)

finally:
    mcp1.close()
