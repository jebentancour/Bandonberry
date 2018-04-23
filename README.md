# Bandonberry
## RaspiATX

El mecanismo de apagado es el de [RaspiATX](https://github.com/LowPowerLab/ATX-Raspi).

Se copia el script de instalación: 
```
sudo wget https://raw.githubusercontent.com/LowPowerLab/ATX-Raspi/master/shutdownchecksetup.sh
```
Se editan los pines a usar ```nano shutdownchecksetup.sh```:
```Python
SHUTDOWN = 12     #GPIO used for shutdown signal
BOOT = 16         #GPIO used for boot signal
```
Se instala eligiendo la opción 1:
```
sudo bash shutdownchecksetup.sh
sudo rm shutdownchecksetup.sh
sudo reboot
```

## Fluidsynth

[FLUIDSYNTH USER MANUAL](https://github.com/FluidSynth/fluidsynth/wiki/UserManual)

Para instalarlo:
```
sudo apt-get install fluidsynth
```
Para usar con las soundfonts de General MIDI:
```
sudo fluidsynth --audio-driver=alsa --gain 3 --verbose /usr/share/sounds/sf2/FluidR3_GM.sf2
```
Para usarlo con las soundfonts nuestras:
```
sudo fluidsynth --audio-driver=alsa --gain 3 --verbose /home/pi/bandonberry/bandoneon_v2.sf2
```
Para que se inicie en el arranque se debe agregar la siguiente línea en ```/etc/rc.local```:
```
sudo fluidsynth -i -s -a alsa -g 3 /home/pi/bandonberry/bandoneon_v2.sf2 &
```

## Rtmidi python

[RTMIDI PYTHON GITHUB](https://github.com/superquadratic/rtmidi-python)

Para instalarlo:
```
sudo apt-get install build-essential
sudo apt-get install libasound2-dev
sudo apt-get install libjack-jackd2-dev
sudo apt-get install python-dev python-pip
sudo pip install rtmidi-python
```

## GPIO expander

El chip [MCP23S17](http://ww1.microchip.com/downloads/en/DeviceDoc/20001952C.pdf) es SPI, se debe habilitar SPI desde ```sudo raspi-config```.

[GUíA ADAFRUIT](https://learn.adafruit.com/adafruits-raspberry-pi-lesson-4-gpio-setup/configuring-spi).

La librería [RPiMCP23S17](https://github.com/petrockblog/RPi-MCP23S17/blob/master/RPiMCP23S17/MCP23S17.py) no fincionó.

Basados en esa librería se creó [BDN_MCP23S17.py](https://github.com/jebentancour/Bandonberry/blob/master/BDN_MCP23S17.py).

Requisitos previos:
```
sudo apt-get install python-dev python-pip
sudo pip install RPi.GPIO
sudo pip install spidev
```

Para usarlo (tiene que estar en el mismo directorio):
```Python
from BDN_MCP23S17 import MCP23S17
```
