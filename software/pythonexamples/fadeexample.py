#!/usr/bin/python
import uberbus.moodlamp
import time
import sys

lamp = sys.argv[1]
t = float(sys.argv[2])

a = uberbus.moodlamp.Moodlamp(lamp)
a.connect();
while 1:
    a.timedfade(255,0,0,t,False)
    time.sleep(t)
    a.timedfade(0,255,0,t,False)
    time.sleep(t)

