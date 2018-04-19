from RPiMCP23S17.MCP23S17 import MCP23S17
import time

mcp1 = MCP23S17(bus=0x00, ce=0x01, deviceID=0x00)
mcp1.open()

for x in range(0, 16):
    mcp1.setDirection(x, mcp1.DIR_OUTPUT)

print "Starting blinky on all pins (CTRL+C to quit)"
while (True):
    for x in range(0, 16):
        mcp1.digitalWrite(x, MCP23S17.LEVEL_HIGH)
    time.sleep(1)

    for x in range(0, 16):
        mcp1.digitalWrite(x, MCP23S17.LEVEL_LOW)
    time.sleep(1)
