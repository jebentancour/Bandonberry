#!/usr/bin/python
# -*- coding: utf-8 -*-

## Importo librerias
import Adafruit_BMP.BMP085 as BMP085
import rtmidi_python as rtmidi
import time

## Varias opciones
sensor = BMP085.BMP085(mode=BMP085.BMP085_STANDARD)
## sensor = BMP085.BMP085(mode=BMP085.BMP085_ULTRALOWPOWER)
## sensor = BMP085.BMP085(mode=BMP085.BMP085_HIGHRES)
## sensor = BMP085.BMP085(mode=BMP085.BMP085_ULTRAHIGHRES)

## Etiquetas MIDI
CONTROL = 0xB0
CUSTOM = 0x09
VOLUME = 0x07

## Puertos MIDI
SYNTH_PORT_NAME = "FLUID"
KEYPAD_PORT_NAME = "KEYPAD"

## Presión atmosférica
media = 101800

## Sensibilidad (menor más sensible)
sensibilidad = 5

## TOPE
tope = 127

midi_out1 = rtmidi.MidiOut()
for port_name in midi_out1.ports:
    if SYNTH_PORT_NAME in port_name:
        midi_out1.open_port(port_name) # Me conecto con el puerto MIDI del sintetizador
        print "Puerto sintetizador encontrado"
        midi_out2 = rtmidi.MidiOut()
        for port_name in midi_out2.ports:
            if KEYPAD_PORT_NAME in port_name:
                midi_out2.open_port(port_name) # Me conecto con el puerto MIDI del teclado
                print "Puerto KeyPad encontrado"
                presion= sensor.read_pressure()
                if (presion-media)>0:
                    estado= 1 ## Cerrando
                else:
                    estado= 0 ## Abriendo
                while True:
                    presion= sensor.read_pressure()
                    if (presion - media)>0 and estado==0:
                        print "Cerrando"
                        estado=1
                        midi_out2.send_message([CONTROL, CUSTOM, 0x01]) # Fuelle cerrando
                    elif (presion -media)<0 and estado==1:
                        print "Abriendo"
                        estado=0
                        midi_out2.send_message([CONTROL, CUSTOM, 0x00]) # Fuelle abriendo
                    else:
                        pass
                    #print('Pressure = {0:0.2f} Pa'.format(((abs(presion-media))/8)))
                    if (abs(presion-media)/sensibilidad) < 127:
                        midi_out1.send_message([CONTROL, VOLUME, (abs(presion-media))/sensibilidad])
                    else:
                        midi_out1.send_message([CONTROL, VOLUME, tope])
                    #time.sleep(0.002)
