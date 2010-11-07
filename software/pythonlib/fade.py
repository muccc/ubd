#!/usr/bin/python
import libml
import time
import sys

lamp = sys.argv[1]
a = libml.Moodlamp(lamp)
a.connect();
while 1:
    a.fade(0,255,0,300)
    time.sleep(2)
    a.fade(255,0,0,300)
    time.sleep(2)

