#!/usr/bin/python

import rtmidi_python as rtmidi
import time

NOTE_ON = 0x90
NOTE_OFF = 0x80
CONTROL = 0xB0
VOLUME = 0x07
BALANCE = 0x0A

LEFT = 0x00
RIGHT = 0x7F
CENTER = 0x40

MIDI_PORT_NAME = "FLUID"
NOTE = 60
VELOCITY = 127
DURATION = 1

midi_out = rtmidi.MidiOut()

for port_name in midi_out.ports:
    if MIDI_PORT_NAME in port_name:
        midi_out.open_port(port_name)
        
        midi_out.send_message([CONTROL | 0x00, VOLUME, 0xFF])
        midi_out.send_message([CONTROL | 0x01, VOLUME, 0xFF])
        midi_out.send_message([CONTROL | 0x02, VOLUME, 0xFF])

        midi_out.send_message([CONTROL | 0x00, BALANCE, CENTER])
        midi_out.send_message([CONTROL | 0x01, BALANCE, LEFT])
        midi_out.send_message([CONTROL | 0x02, BALANCE, RIGHT])

        midi_out.send_message([NOTE_ON | 0x00, NOTE, VELOCITY])
        time.sleep(DURATION)
        midi_out.send_message([NOTE_OFF | 0x00, NOTE, VELOCITY])
        time.sleep(DURATION)


        midi_out.send_message([NOTE_ON | 0x01, NOTE, VELOCITY])
        time.sleep(DURATION)
        midi_out.send_message([NOTE_OFF | 0x01, NOTE, VELOCITY])
        time.sleep(DURATION)

        midi_out.send_message([NOTE_ON | 0x02, NOTE, VELOCITY])
        time.sleep(DURATION)
        midi_out.send_message([NOTE_OFF | 0x02, NOTE, VELOCITY])
        time.sleep(DURATION)
