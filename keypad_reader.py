#!/usr/bin/python

from BDN_MCP23S17 import MCP23S17
import rtmidi_python as rtmidi
import time

MIDI_PORT_NAME = "FLUID"
VELOCITY = 127
NOTE_ON = 0x90
NOTE_OFF = 0x80

COLUMS = 6
ROWS = 8
NUM_OF_READS = 2
DEBOUNCE_DELAY = 0.001

current_dir = 1 # 0 = Abriendo, 1 = Cerrando

right_notes_matrix = [[[0, 0] for x in range(ROWS)] for y in range(COLUMS)]
# Mano Derecha
right_notes_matrix[0][0] = [81, 80]
right_notes_matrix[0][1] = [85, 88]
right_notes_matrix[0][2] = [79, 75]
right_notes_matrix[0][3] = [78, 80]
right_notes_matrix[0][4] = [75, 76]
right_notes_matrix[0][5] = [77, 77]
right_notes_matrix[0][6] = [63, 63]
right_notes_matrix[0][7] = [57, 58]
right_notes_matrix[1][0] = [74, 76]
right_notes_matrix[1][1] = [80, 81]
right_notes_matrix[1][2] = [83, 85]
right_notes_matrix[1][3] = [71, 73]
right_notes_matrix[1][4] = [68, 69]
right_notes_matrix[1][5] = [70, 64]
right_notes_matrix[1][6] = [65, 65]
right_notes_matrix[1][7] = [57, 57]
right_notes_matrix[2][0] = [69, 71]
right_notes_matrix[2][1] = [72, 74]
right_notes_matrix[2][2] = [76, 79]
right_notes_matrix[2][3] = [66, 67]
right_notes_matrix[2][4] = [73, 78]
right_notes_matrix[2][5] = [64, 66]
right_notes_matrix[2][6] = [59, 59]
right_notes_matrix[3][0] = [82, 70]
right_notes_matrix[3][1] = [84, 72]
right_notes_matrix[3][2] = [86, 87]
right_notes_matrix[3][3] = [67, 68]
right_notes_matrix[3][4] = [62, 61]
right_notes_matrix[3][5] = [60, 62]
right_notes_matrix[4][0] = [90, 82]
right_notes_matrix[4][1] = [88, 84]
right_notes_matrix[4][2] = [87, 86]
right_notes_matrix[4][3] = [93, 91]
right_notes_matrix[4][4] = [61, 60]
right_notes_matrix[5][0] = [92, 92]
right_notes_matrix[5][1] = [91, 90]
right_notes_matrix[5][2] = [89, 89]
right_notes_matrix[5][3] = [94, 93]

prev_data = [0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF]

try:
    midi_out = rtmidi.MidiOut()
    port_found = False
    for port_name in midi_out.ports:
        if MIDI_PORT_NAME in port_name:
            midi_out.open_port(port_name)
            print "MIDI port found."
            port_found = True
            for note in range(128):
                midi_out.send_message([NOTE_OFF, note, VELOCITY])
    if not port_found:
        print "MIDI port not found, exit."
        sys.exit()

    mcp1 = MCP23S17(ce=1)
    mcp1.open()
    mcp1.setDirPORTA(0xFF)
    mcp1.setDirPORTB(0xC0)
    mcp1.setPullupPORTA(0xFF)
    mcp1.setPullupPORTB(0xC0)

    while (True):
        new_data = [0x00, 0x00, 0x00, 0x00, 0x00, 0x00]
        for current_col in range(COLUMS):
            mcp1.writePORTB(~(0x01 << current_col))
            for i in range(NUM_OF_READS):
                new_data[current_col] |= mcp1.readPORTA()
                time.sleep(DEBOUNCE_DELAY)
            if prev_data[current_col] != new_data[current_col]:
                for current_row in range(ROWS):
                    new_bit = (new_data[current_col] >> current_row) & 0x01
                    prev_bit = (prev_data[current_col] >> current_row) & 0x01
                    if new_bit != prev_bit:
                        if new_bit:
                            note = right_notes_matrix[current_col][current_row][current_dir]
                            midi_out.send_message([NOTE_OFF, note, VELOCITY])
                            print 'Note OFF [{0}][{1}] {2}'.format(current_col, current_row, note)
                        else:
                            note = right_notes_matrix[current_col][current_row][current_dir]
                            midi_out.send_message([NOTE_ON, note, VELOCITY])
                            print 'Note ON [{0}][{1}] {2}'.format(current_col, current_row, note)
        prev_data = new_data

finally:
    mcp1.close()