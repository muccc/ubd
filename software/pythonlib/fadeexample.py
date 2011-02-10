#!/usr/bin/python
import libml
import time
import sys

lamp = sys.argv[1]
t = float(sys.argv[2])

a = libml.Moodlamp(lamp)
a.connect();
while 1:
    a.timedfade(0,0,255,t)
    time.sleep(t)
    a.timedfade(0,255,0,t)
    time.sleep(t)
    a.timedfade(255,0,0,t)
    time.sleep(t)

