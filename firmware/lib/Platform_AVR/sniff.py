import serial
import time

ser = serial.Serial("/dev/ttyUSB0",19200)

t = 0
while(1):
    c = ser.read(1);
    if time.time() - t > 0.02:
        print
        print "%.2f %s %s"%(time.time(),time.ctime(), hex(ord(c))),
    else:
        print hex(ord(c)),
    t = time.time()
