#!/usr/bin/python

## Importo librerias
import Adafruit_BMP.BMP085 as BMP085
import rtmidi_python as rtmidi
import time

#import RPi.GPIO as GPIO
#LED_PIN = 27
#GPIO.setmode(GPIO.BCM)
#GPIO.setup(LED_PIN, GPIO.OUT)
#GPIO.output(LED_PIN, GPIO.LOW)
#led_state = False

## Oppciones de muestreo
sensor = BMP085.BMP085(mode=BMP085.BMP085_STANDARD)
## sensor = BMP085.BMP085(mode=BMP085.BMP085_ULTRALOWPOWER)
## sensor = BMP085.BMP085(mode=BMP085.BMP085_HIGHRES)
## sensor = BMP085.BMP085(mode=BMP085.BMP085_ULTRAHIGHRES)

## Etiquetas MIDI
CONTROL = 0xB0
VOLUME = 0x07

## Puertos MIDI
SYNTH_PORT_NAME = "FLUID"
USB_PORT_NAME = "f_midi"

try:
    ## Presion atmosferica
    presion = 0
    for i in range(10):
        presion += sensor.read_pressure()
    media = presion / 10
    print('Presion atm. = {0:0.2f} Pa'.format(media))

    # Me conecto con el puerto MIDI del sintetizador y USB
    midi_out = rtmidi.MidiOut()
    port_found = False
    while not port_found:
        for port_name in midi_out.ports:
            if SYNTH_PORT_NAME in port_name:
                midi_out.open_port(port_name)
                print "Puerto sintetizador encontrado"
                port_found = True

    midi_usb_out = rtmidi.MidiOut()
    port_found = False
    while not port_found:
        for port_name in midi_usb_out.ports:
            if USB_PORT_NAME in port_name:
                midi_usb_out.open_port(port_name)
                print "Puerto USB MIDI encontrado"
                port_found = True

    # max_value = 0;

    while True:

        #if led_state:
            #GPIO.output(LED_PIN, GPIO.HIGH)
            #led_state = False
        #else:
            #GPIO.output(LED_PIN, GPIO.LOW)
            #led_state = True

        # Mido
        presion = sensor.read_pressure()
        # print('Presion = {0:0.2f} Pa'.format(presion))

        # Calculo
        value = abs(presion - media)
        value = (value * 127) / 150
        # print('Volume = {0:0.2f}'.format(value))

        # if value > max_value:
            # max_value = value
            # print('Max volume = {0:0.2f}'.format(max_value))

        # Checkeo extremos
        if value > 127:
            value = 127

        if (presion - media) > 0:
            # Cerrando
            midi_out.send_message([CONTROL | 0x00, VOLUME, 0x00])  # Mano derecha abriendo
            midi_out.send_message([CONTROL | 0x01, VOLUME, value]) # Mano derecha cerrando
            midi_out.send_message([CONTROL | 0x02, VOLUME, 0x00])  # Mano izquierda abriendo
            midi_out.send_message([CONTROL | 0x03, VOLUME, value]) # Mano izquierda cerrando
            # MIDI USB
            midi_usb_out.send_message([CONTROL | 0x00, VOLUME, 0x00])  # Mano derecha abriendo
            midi_usb_out.send_message([CONTROL | 0x01, VOLUME, value]) # Mano derecha cerrando
            midi_usb_out.send_message([CONTROL | 0x02, VOLUME, 0x00])  # Mano izquierda abriendo
            midi_usb_out.send_message([CONTROL | 0x03, VOLUME, value]) # Mano izquierda cerrando
        elif (presion - media) < 0:
            # Abriendo
            midi_out.send_message([CONTROL | 0x00, VOLUME, value]) # Mano derecha abriendo
            midi_out.send_message([CONTROL | 0x01, VOLUME, 0x00])  # Mano derecha cerrando
            midi_out.send_message([CONTROL | 0x02, VOLUME, value]) # Mano izquierda abriendo
            midi_out.send_message([CONTROL | 0x03, VOLUME, 0x00])  # Mano izquierda cerrando
            # MIDI USB
            midi_usb_out.send_message([CONTROL | 0x00, VOLUME, value]) # Mano derecha abriendo
            midi_usb_out.send_message([CONTROL | 0x01, VOLUME, 0x00])  # Mano derecha cerrando
            midi_usb_out.send_message([CONTROL | 0x02, VOLUME, value]) # Mano izquierda abriendo
            midi_usb_out.send_message([CONTROL | 0x03, VOLUME, 0x00])  # Mano izquierda cerrando
        else:
            midi_out.send_message([CONTROL | 0x00, VOLUME, 0x00]) # Mano derecha abriendo
            midi_out.send_message([CONTROL | 0x01, VOLUME, 0x00]) # Mano derecha cerrando
            midi_out.send_message([CONTROL | 0x02, VOLUME, 0x00]) # Mano izquierda abriendo
            midi_out.send_message([CONTROL | 0x03, VOLUME, 0x00]) # Mano izquierda cerrando
            # MIDI USB
            midi_usb_out.send_message([CONTROL | 0x00, VOLUME, 0x00]) # Mano derecha abriendo
            midi_usb_out.send_message([CONTROL | 0x01, VOLUME, 0x00]) # Mano derecha cerrando
            midi_usb_out.send_message([CONTROL | 0x02, VOLUME, 0x00]) # Mano izquierda abriendo
            midi_usb_out.send_message([CONTROL | 0x03, VOLUME, 0x00]) # Mano izquierda cerrando

        # Calculo la media
        media = media * 0.999 + presion * 0.001

        time.sleep(0.001)

finally:
    midi_out.close_port()
    midi_usb_out.close_port()
