#!/usr/bin/python
import uberbus.moodlamp
import uberbus.switch
import time
import sys

lamp = sys.argv[1]
switch = sys.argv[2]

a = uberbus.moodlamp.Moodlamp(lamp,True)
s = uberbus.switch.Switch(switch)

a.connect()
s.connect()
s.listen()

while True:
    rc = s.receiveStatus()
    if rc == 'b0':
        a.timedfade(0,0,255,1)
    elif rc == 'B0':
        a.timedfade(255,0,0,1)

