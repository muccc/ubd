#!/usr/bin/python
import libml
import time
import sys

lamp = sys.argv[1]
a = libml.Moodlamp(lamp)
a.connect();
while 1:
    a.fadems(0,0,255,3000)
    time.sleep(3)
    a.fadems(0,255,0,3000)
    time.sleep(3)
    a.fadems(255,0,0,3000)
    time.sleep(3)

