#!/usr/bin/python
import uberbus.moodlamp
import time
import sys

lamp = sys.argv[1]
a = uberbus.moodlamp.Moodlamp(lamp)
t = float(sys.argv[2])

a.connect();
while 1:
    a.setcolor(255,0,0)
    time.sleep(t)
    a.setcolor(0,255,0)
    time.sleep(t)

