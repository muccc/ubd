#!/usr/bin/python
import libml
import time
import sys

lamp = sys.argv[1]
a = libml.Moodlamp(lamp)
a.connect();
while 1:
    a.timedfade(0,0,255,3)
    time.sleep(3)
    a.timedfade(0,255,0,3)
    time.sleep(3)
    a.timedfade(255,0,0,3)
    time.sleep(3)

