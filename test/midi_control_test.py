#!/usr/bin/python

import rtmidi_python as rtmidi
import time

MIDI_PORT_NAME = "FLUID"

NOTE = 60
VELOCITY = 120
DURATION = 0.5

NOTE_ON = 0x90
NOTE_OFF = 0x80
CONTROL = 0xB0

VOLUME = 0x07
EXPRESSION = 0x0B

midi_out = rtmidi.MidiOut()

for port_name in midi_out.ports:
    if MIDI_PORT_NAME in port_name:
        midi_out.open_port(port_name)

        midi_out.send_message([CONTROL, VOLUME, 127])
        midi_out.send_message([NOTE_ON, NOTE, VELOCITY])
        time.sleep(DURATION)
        midi_out.send_message([CONTROL, VOLUME, 64])
        time.sleep(DURATION)
        midi_out.send_message([NOTE_OFF, NOTE, VELOCITY])
