# Bandonberry
## Fluidsynth
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
## Rtmidi python
```
sudo apt-get install build-essential
sudo apt-get install libasound2-dev
sudo apt-get install libjack-jackd2-dev
sudo apt-get install python-dev python-pip
sudo pip install rtmidi-python
```
## GPIO expander
El chip MCP23S17 es SPI, se debe habilitar SPI desde ```sudo raspi-config```.

[Guía Adafruit](https://learn.adafruit.com/adafruits-raspberry-pi-lesson-4-gpio-setup/configuring-spi).

La libreria Python [RPiMCP23S17](https://pypi.org/project/RPiMCP23S17/) no finciono asi como esta, se le hicieron modificaciones para que funcionara.
Se creo la libreria BDN_MCP23S17.

```
sudo apt-get install python-dev python-pip
sudo pip install RPi.GPIO
sudo pip install spidev
```
