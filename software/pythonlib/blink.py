import libml
import time
import sys
a = libml.Moodlamp("newlamp,local")

if a.connect() != True:
    print "could not open a connection"
    sys.exit()

while 1:
    a.setcolor(0,0,255)
    time.sleep(0.5)
    a.setcolor(0,0,0)
    time.sleep(0.5)

