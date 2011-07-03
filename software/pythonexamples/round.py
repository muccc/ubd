#!/usr/bin/python
import libml
import time
import sys

#lamps = ['lamp63.local','lamp64.local','lamp62.local','lamp65.local','lamp69.local','lamp67.local','lamp68.local','lamp66.local']
lamps = ['lamp58.local', 'lamp5A.local','lamp6B.local', 'lamp5B.local', 'lamp59.local']
t = float(sys.argv[1])

lamps = map( lambda x: libml.Moodlamp(x), lamps)
for lamp in lamps:
    lamp.connect()
index = 0
r = 255
g = 0
b = 0

while 1:
    r = 255
    b = 0
    lamps[index].timedfade(r,g,b,t)
    time.sleep(t)
    r = 0
    b = 255
    lamps[index].timedfade(r,g,b,t*3)
    time.sleep(t)
    index+=1
    if index == len(lamps):
        index = 0
