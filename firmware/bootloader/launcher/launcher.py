import serial
import sys
import time

if len(sys.argv) < 2:
    print 'Usage: %s <serial device>'%sys.argv[0]
    sys.exit(1)

device = sys.argv[1]

print 'Opening serial device %s.'%device
try:
    ser = serial.Serial(device, 115200)
except:
    print 'Serial device does not exist, waiting for it to be plugged in.'
    while True:
        try:
            ser = serial.Serial(device, 115200)
            break
        except:
            time.sleep(0.01)

ser.setTimeout(0.0002)

print 'Sending bootloader entry command (p).'
print 'You may now plug in the target device.'
ser.write('\\R\\1R\\2')
while True:
    ser.write('p')
    d = ser.read(1)
    if d:
        if d == 'S':
            print 'Bootloader running.'
            sys.exit(0)
