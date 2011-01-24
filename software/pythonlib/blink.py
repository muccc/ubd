#!/usr/bin/python
import libml
import time
import sys

lamp = sys.argv[1]
a = libml.Moodlamp(lamp)
#a = libml.Moodlamp("2001:a60:e801:1001:1::3")

#if a.connect() != True:
#    print "could not open a connection"
#    sys.exit()
a.connect();
while 1:
    a.setcolor(0,255,255)
    time.sleep(0.5)
    a.setcolor(0,0,0)
    time.sleep(0.5)

