#!/usr/bin/python
import uberbus.moodlamp
import time
import sys

lamp = sys.argv[1]
a = uberbus.moodlamp.Moodlamp(lamp)
t = float(sys.argv[2])
#a = libml.Moodlamp("2001:a60:e801:1001:1::3")

#if a.connect() != True:
#    print "could not open a connection"
#    sys.exit()
a.connect();
while 1:
    a.setcolor(0,255,255)
    time.sleep(t)
    a.setcolor(0,0,0)
    time.sleep(t)

