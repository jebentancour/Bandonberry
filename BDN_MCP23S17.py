#!/usr/bin/python

import spidev

class MCP23S17(object):
    """This class provides an abstraction of the GPIO expander MCP23S17
    for the Raspberry Pi.
    It is depndent on the Python packages spidev, which can
    be get from https://pypi.python.org/pypi/spidev.
    """
    DIR_INPUT = 1
    DIR_OUTPUT = 0
    PULLUP_ENABLED = 1
    PULLUP_DISABLED = 0
    LEVEL_LOW = 0
    LEVEL_HIGH = 1

    """Register addresses (ICON.BANK = 0) as documentined in the technical data sheet at
    http://ww1.microchip.com/downloads/en/DeviceDoc/21952b.pdf
    """
    MCP23S17_IODIRA = 0x00
    MCP23S17_IODIRB = 0x01
    MCP23S17_IPOLA = 0x02
    MCP23S17_IPOLB = 0x03
    MCP23S17_GPIOA = 0x12
    MCP23S17_GPIOB = 0x13
    MCP23S17_OLATA = 0x14
    MCP23S17_OLATB = 0x15
    MCP23S17_IOCON = 0x0A
    MCP23S17_GPPUA = 0x0C
    MCP23S17_GPPUB = 0x0D

    """Bit field flags as documentined in the technical data sheet at
    http://ww1.microchip.com/downloads/en/DeviceDoc/21952b.pdf
    """
    IOCON_UNUSED = 0x01
    IOCON_INTPOL = 0x02
    IOCON_ODR = 0x04
    IOCON_HAEN = 0x08
    IOCON_DISSLW = 0x10
    IOCON_SEQOP = 0x20
    IOCON_MIRROR = 0x40
    IOCON_BANK_MODE = 0x80

    IOCON_INIT = 0x00 # IOCON_BANK_MODE = 0, IOCON_HAEN = 0 address pins disabled

    MCP23S17_CMD_WRITE = 0x40
    MCP23S17_CMD_READ = 0x41

    def __init__(self, bus=0, ce=0, deviceID=0x00):
        """Constructor
        Initializes all attributes with 0.
        Keyword arguments:
        bus -- The SPI bus number
        ce -- The chip-enable number for the SPI
        deviceID -- The device ID of the component, i.e., the hardware address (default 0.0)
        """
        self.spi = spidev.SpiDev()
        self.bus = bus
        self.ce = ce
        self.deviceID = deviceID
        self._GPIOA = 0x00
        self._GPIOB = 0x00
        self._IODIRA = 0xFF
        self._IODIRB = 0xFF
        self._GPPUA = 0x00
        self._GPPUB = 0x00
        self.isInitialized = False

    def open(self):
        """Initializes the MCP23S17 with hardware-address access
        and sequential operations mode.
        """
        self.spi.open(self.bus, self.ce)
        self.spi.max_speed_hz = 10000000
        self.isInitialized = True

        self._writeRegister(MCP23S17.MCP23S17_IOCON, MCP23S17.IOCON_INIT)

    def close(self):
        """Closes the SPI connection that the MCP23S17 component is using.
        """
        self.spi.close()
        self.isInitialized = False

    def setPullupMode(self, pin, mode):
        """Enables or disables the pull-up mode for input pins.
        Parameters:
        pin -- The pin index (0 - 15)
        mode -- The pull-up mode (MCP23S17.PULLUP_ENABLED, MCP23S17.PULLUP_DISABLED)
        """
        assert(pin < 16)
        assert((mode == MCP23S17.PULLUP_ENABLED)
               or (mode == MCP23S17.PULLUP_DISABLED))
        assert(self.isInitialized)

        if (pin < 8):
            register = MCP23S17.MCP23S17_GPPUA
            data = self._GPPUA
            noshifts = pin
        else:
            register = MCP23S17.MCP23S17_GPPUB
            noshifts = pin & 0x07
            data = self._GPPUB

        if (mode == MCP23S17.PULLUP_ENABLED):
            data |= (1 << noshifts)
        else:
            data &= (~(1 << noshifts))

        self._writeRegister(register, data)

        if (pin < 8):
            self._GPPUA = data
        else:
            self._GPPUB = data

    def setDirection(self, pin, direction):
        """Sets the direction for a given pin.
        Parameters:
        pin -- The pin index (0 - 15)
        direction -- The direction of the pin (MCP23S17.DIR_INPUT, MCP23S17.DIR_OUTPUT)
        """
        assert (pin < 16)
        assert ((direction == MCP23S17.DIR_INPUT)
                or (direction == MCP23S17.DIR_OUTPUT))
        assert(self.isInitialized)

        if (pin < 8):
            register = MCP23S17.MCP23S17_IODIRA
            data = self._IODIRA
            noshifts = pin
        else:
            register = MCP23S17.MCP23S17_IODIRB
            noshifts = pin & 0x07
            data = self._IODIRB

        if (direction == MCP23S17.DIR_INPUT):
            data |= (1 << noshifts)
        else:
            data &= (~(1 << noshifts))

        self._writeRegister(register, data)

        if (pin < 8):
            self._IODIRA = data
        else:
            self._IODIRB = data

    def digitalRead(self, pin):
        """Reads the logical level of a given pin.
        Parameters:
        pin -- The pin index (0 - 15)
        Returns:
         - MCP23S17.LEVEL_LOW, if the logical level of the pin is low,
         - MCP23S17.LEVEL_HIGH, otherwise.
        """
        assert(self.isInitialized)
        assert (pin < 16)

        if (pin < 8):
            self._GPIOA = self._readRegister(MCP23S17.MCP23S17_GPIOA)
            if ((self._GPIOA & (1 << pin)) != 0):
                return MCP23S17.LEVEL_HIGH
            else:
                return MCP23S17.LEVEL_LOW
        else:
            self._GPIOB = self._readRegister(MCP23S17.MCP23S17_GPIOB)
            pin &= 0x07
            if ((self._GPIOB & (1 << pin)) != 0):
                return MCP23S17.LEVEL_HIGH
            else:
                return MCP23S17.LEVEL_LOW

    def digitalWrite(self, pin, level):
        """Sets the level of a given pin.
        Parameters:
        pin -- The pin idnex (0 - 15)
        level -- The logical level to be set (MCP23S17.LEVEL_LOW, MCP23S17.LEVEL_HIGH)
        """
        assert(self.isInitialized)
        assert(pin < 16)
        assert((level == MCP23S17.LEVEL_HIGH) or (level == MCP23S17.LEVEL_LOW))

        if (pin < 8):
            register = MCP23S17.MCP23S17_GPIOA
            data = self._GPIOA
            noshifts = pin
        else:
            register = MCP23S17.MCP23S17_GPIOB
            noshifts = pin & 0x07
            data = self._GPIOB

        if (level == MCP23S17.LEVEL_HIGH):
            data |= (1 << noshifts)
        else:
            data &= (~(1 << noshifts))

        self._writeRegister(register, data)

        if (pin < 8):
            self._GPIOA = data
        else:
            self._GPIOB = data

    def setDirPORTA(self, data):
        assert(self.isInitialized)

        self._writeRegister(MCP23S17.MCP23S17_IODIRA, data)
        self._IODIRA = data

    def setDirPORTB(self, data):
        assert(self.isInitialized)

        self._writeRegister(MCP23S17.MCP23S17_IODIRB, data)
        self._IODIRA = data

    def setPullupPORTA(self, data):
        assert(self.isInitialized)

        self._writeRegister(MCP23S17.MCP23S17_GPPUA, data)
        self._GPPUA = data

    def setPullupPORTB(self, data):
        assert(self.isInitialized)

        self._writeRegister(MCP23S17.MCP23S17_GPPUB, data)
        self._GPPUB = data

    def readPORTA(self):
        assert(self.isInitialized)

        data = self._readRegister(MCP23S17.MCP23S17_GPIOA)
        self._GPIOA = data
        return data

    def readPORTB(self):
        assert(self.isInitialized)

        data = self._readRegister(MCP23S17.MCP23S17_GPIOB)
        self._GPIOB = data
        return data

    def writePORTA(self,data):
        assert(self.isInitialized)

        self._writeRegister(MCP23S17.MCP23S17_GPIOA, data)
        self._GPIOA = data

    def writePORTB(self,data):
        assert(self.isInitialized)

        self._writeRegister(MCP23S17.MCP23S17_GPIOB, data)
        self._GPIOB = data

    def writeGPIO(self, data):
        """Sets the data port value for all pins.
        Parameters:
        data - The 16-bit value to be set.
        """
        assert(self.isInitialized)

        self._GPIOA = (data & 0xFF)
        self._GPIOB = (data >> 8)
        self._writeRegisterWord(MCP23S17.MCP23S17_GPIOA, data)

    def readGPIO(self):
        """Reads the data port value of all pins.
        Returns:
         - The 16-bit data port value
        """
        assert(self.isInitialized)

        data = self._readRegisterWord(MCP23S17.MCP23S17_GPIOA)
        self._GPIOA = (data & 0xFF)
        self._GPIOB = (data >> 8)
        return data

    def _writeRegister(self, register, value):
        assert(self.isInitialized)

        command = MCP23S17.MCP23S17_CMD_WRITE | ((self.deviceID) << 1)
        self.spi.xfer2([command, register, value])

    def _readRegister(self, register):
        assert(self.isInitialized)

        command = MCP23S17.MCP23S17_CMD_READ | ((self.deviceID) << 1)
        data = self.spi.xfer2([command, register, 0])
        return data[2]

    def _readRegisterWord(self, register):
        assert(self.isInitialized)

        buffer = [0, 0]
        buffer[0] = self._readRegister(register)
        buffer[1] = self._readRegister(register + 1)
        return ((buffer[1] << 8) | buffer[0])

    def _writeRegisterWord(self, register, data):
        assert(self.isInitialized)

        self._writeRegister(register, data & 0xFF)
        self._writeRegister(register + 1, data >> 8)
