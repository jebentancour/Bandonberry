#!/usr/bin/python

import rtmidi_python as rtmidi
import time

CONTROL = 0xB0
CUSTOM = 0x09
VOLUME = 0x07

SYNTH_PORT_NAME = "FLUID"

midi_out1 = rtmidi.MidiOut()
for port_name in midi_out1.ports:
    if SYNTH_PORT_NAME in port_name:
        midi_out1.open_port(port_name) # Me conecto con el puerto MIDI del sintetizador
        print "cambiando volumen"

        # Para modular la amplitud (el ultimo valor debe estar entre 0 y 127)
        print "0"
        midi_out1.send_message([CONTROL, VOLUME, 0])
        time.sleep(1)
        print "64"
        midi_out1.send_message([CONTROL, VOLUME, 64])
        time.sleep(1)
        print "128"
        midi_out1.send_message([CONTROL, VOLUME, 127])
        time.sleep(1)

KEYPAD_PORT_NAME = "KEYPAD"

midi_out2 = rtmidi.MidiOut()
for port_name in midi_out2.ports:
    if KEYPAD_PORT_NAME in port_name:
        midi_out2.open_port(port_name) # Me conecto con el puerto MIDI del teclado
        print "cambiando direccion"

        # Para cambiar la direccion del fuelle
        print "abriendo"
        midi_out2.send_message([CONTROL, CUSTOM, 0x00]) # Fuelle abriendo
        time.sleep(2)
        print "cerrando"
        midi_out2.send_message([CONTROL, CUSTOM, 0x01]) # Fuelle cerrando
        time.sleep(2)
