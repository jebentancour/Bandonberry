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
presion = 0
for i in range(10):
    presion += sensor.read_pressure()
media = presion / 10
print('Presion atm. = {0:0.2f} Pa'.format(media))

## Sensibilidad (menor más sensible)
sensibilidad = 1

## TOPE
tope = 127

## MINIMO
min = 0

# Me conecto con el puerto MIDI del sintetizador
midi_out1 = rtmidi.MidiOut()
port_found = False
while not port_found:
    for port_name in midi_out1.ports:
        if SYNTH_PORT_NAME in port_name:
            midi_out1.open_port(port_name)
            print "Puerto sintetizador encontrado"
            port_found = True

# Me conecto con el puerto MIDI del teclado
midi_out2 = rtmidi.MidiOut()
port_found = False
while not port_found:
    for port_name in midi_out2.ports:
        if KEYPAD_PORT_NAME in port_name:
            midi_out2.open_port(port_name)
            print "Puerto KeyPad encontrado"
            port_found = True

# Comienzan las lecturas del sensor
presion = sensor.read_pressure()
if (presion - media) > 0:
    estado = 1 ## Cerrando
else:
    estado = 0 ## Abriendo
while True:
    presion = (presion + sensor.read_pressure())/2
    print('Presion = {0:0.2f} Pa'.format(presion))
    if (presion - media) > 0 and estado == 0:
        print "Cerrando"
        estado = 1
        midi_out2.send_message([CONTROL, CUSTOM, 0x01]) # Fuelle cerrando
    elif (presion -media) < 0 and estado == 1:
        print "Abriendo"
        estado = 0
        midi_out2.send_message([CONTROL, CUSTOM, 0x00]) # Fuelle abriendo
    else:
        pass
    value = abs(presion-media)/sensibilidad
    print('Volume = {0:0.2f}'.format(((abs(presion-media))/sensibilidad)))
    if value < min:
        midi_out1.send_message([CONTROL, VOLUME, 0])
    else:
        if (value) < 127:
            midi_out1.send_message([CONTROL, VOLUME, value])
        else:
            midi_out1.send_message([CONTROL, VOLUME, tope])
    #time.sleep(0.2)
    media = media * 0.999 + presion * 0.001
