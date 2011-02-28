#!/usr/bin/python
import libml
import libswitch
import time
import sys

lamp = sys.argv[1]
switch = sys.argv[2]

a = libml.Moodlamp(lamp,True)
s = libswitch.Switch(switch)

a.connect()
s.connect()
s.listen()

while True:
    rc = s.receiveStatus()
    if rc == 'b0':
        a.timedfade(0,0,255,1)
    elif rc == 'B0':
        a.timedfade(255,0,0,1)

