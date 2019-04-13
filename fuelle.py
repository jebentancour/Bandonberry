#!/usr/bin/python

## Importo librerias
import Adafruit_BMP.BMP085 as BMP085
import rtmidi_python as rtmidi
import time

## Varias opciones de muestreo
sensor = BMP085.BMP085(mode=BMP085.BMP085_STANDARD)
## sensor = BMP085.BMP085(mode=BMP085.BMP085_ULTRALOWPOWER)
## sensor = BMP085.BMP085(mode=BMP085.BMP085_HIGHRES)
## sensor = BMP085.BMP085(mode=BMP085.BMP085_ULTRAHIGHRES)

## Etiquetas MIDI
CONTROL = 0xB0
VOLUME = 0x07

## Puertos MIDI
SYNTH_PORT_NAME = "FLUID"

## Presión atmosférica
presion = 0
for i in range(10):
    presion += sensor.read_pressure()
media = presion / 10
print('Presion atm. = {0:0.2f} Pa'.format(media))

# Me conecto con el puerto MIDI del sintetizador
midi_out = rtmidi.MidiOut()
port_found = False
while not port_found:
    for port_name in midi_out.ports:
        if SYNTH_PORT_NAME in port_name:
            midi_out.open_port(port_name)
            print "Puerto sintetizador encontrado"
            port_found = True

while True:
    # Mido
    presion = sensor.read_pressure()
    # print('Presion = {0:0.2f} Pa'.format(presion))

    # Calculo
    value = abs(presion - media)
    # print('Volume = {0:0.2f}'.format(value))

    # Checkeo extremos
    if value > 127:
        value = 127;

    if (presion - media) > 0:
        # Cerrando
        midi_out.send_message([CONTROL | 0x00, VOLUME, 0x00])  # Mano derecha abriendo
        midi_out.send_message([CONTROL | 0x01, VOLUME, value]) # Mano derecha cerrando
        midi_out.send_message([CONTROL | 0x02, VOLUME, 0x00])  # Mano izquierda abriendo
        midi_out.send_message([CONTROL | 0x03, VOLUME, value]) # Mano izquierda cerrando
    elif (presion - media) < 0:
        # Abriendo
        midi_out.send_message([CONTROL | 0x00, VOLUME, value]) # Mano derecha abriendo
        midi_out.send_message([CONTROL | 0x01, VOLUME, 0x00])  # Mano derecha cerrando
        midi_out.send_message([CONTROL | 0x02, VOLUME, value]) # Mano izquierda abriendo
        midi_out.send_message([CONTROL | 0x03, VOLUME, 0x00])  # Mano izquierda cerrando
    else:
        midi_out.send_message([CONTROL | 0x00, VOLUME, 0x00]) # Mano derecha abriendo
        midi_out.send_message([CONTROL | 0x01, VOLUME, 0x00]) # Mano derecha cerrando
        midi_out.send_message([CONTROL | 0x02, VOLUME, 0x00]) # Mano izquierda abriendo
        midi_out.send_message([CONTROL | 0x03, VOLUME, 0x00]) # Mano izquierda cerrando

    # Calculo la media
    media = media * 0.999 + presion * 0.001
