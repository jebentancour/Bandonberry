#!/usr/bin/python

import rtmidi_python as rtmidi
import time

def callback(message, time_stamp):
    print message, time_stamp

midi_in = rtmidi.MidiIn("keypad_in_test")
midi_in.callback = callback
midi_in.open_port(0)

while (True):
    # nothing
    time.sleep(1)
