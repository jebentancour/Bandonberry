#!/usr/bin/python

from BDN_MCP23S17 import MCP23S17
import time

mcp1 = MCP23S17(ce=1)
mcp1.open()

for x in range(0, 16):
    mcp1.setDirection(x, mcp1.DIR_OUTPUT)

print "Starting blinky on all pins (CTRL+C to quit)"
while (True):
    for x in range(0, 16):
        mcp1.digitalWrite(x, MCP23S17.LEVEL_HIGH)
    time.sleep(0.2)

    for x in range(0, 16):
        mcp1.digitalWrite(x, MCP23S17.LEVEL_LOW)
    time.sleep(0.2)
