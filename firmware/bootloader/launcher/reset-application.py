import serial
import sys
import time

if len(sys.argv) < 2:
    print 'Usage: %s <serial device>'%sys.argv[0]
    sys.exit(1)

device = sys.argv[1]

print 'Opening serial device %s.'%device
ser = serial.Serial(device, 115200)

print 'Sending command (R).'
ser.write('\\1R\\2')
