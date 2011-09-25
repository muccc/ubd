#!/usr/bin/python
import uberbus.moodlamp
import time
import sys
import random

#lamps = ['lamp63.local','lamp64.local','lamp62.local','lamp65.local','lamp6A.local','lamp69.local','lamp67.local','lamp68.local','lamp66.local','kueche.local']
lamps = ['racklampe5.local', 'racklampe6.local', 'racklampe7.local', 'racklampe0.local', 'racklampe2.local', 'racklampe3.local', 'racklampe4.local', 'racklampe1.local', ]
#lamps = ['laborlampe0.local', 'laborlampe1.local', 'laborlampe2.local', 'laborlampe3.local', 'laborlampe4.local', 'laborlampe5.local', 'laborlampe6.local', 'laborlampe7.local', 'laborlampe8.local', 'laborlampe9.local', 'laborlampe10.local', 'laborlampe11.local','laborlampe12.local','laborlampe13.local']


def getOffset(pos, offset):
    return (pos+offset)%len(lamps)

def getRandom():
    return int(random.random()*255)

def getMean(r,g,b):
    return (r+g+b)/255

def isNear(r1,g1,b1,r2,g2,b2):
    return abs(getMean(r1,g1,b1) - getMean(r2,g2,b2)) < 50

t = float(sys.argv[1])

lamps = map( lambda x: uberbus.moodlamp.Moodlamp(x), lamps)
for lamp in lamps:
    lamp.connect()
index = 0
r1 = getRandom()
g1 = getRandom()
b1 = getRandom()

r2 = getRandom()
g2 = getRandom()
b2 = getRandom()

while 1:
    lamps[index].timedfade(r1,g1,b1,t*7)
    lamps[getOffset(index,-3)].timedfade(r2,g2,b2,t*7)
    time.sleep(t)
    index = getOffset(index,1)
    if index == 0:
        r2 = r1
        g2 = g1
        b2 = b1
        r = getRandom()
        g = getRandom()
        b = getRandom()
        while not isNear(r1,g1,b1,r,g,b):
            r = getRandom()
            g = getRandom()
            b = getRandom()
        r1 = r
        g1 = g
        b1 = b
