#!/usr/bin/python
import libml
import time
import sys
import random

lamps = ['lamp63.local','lamp64.local','lamp62.local','lamp65.local','lamp6A.local','lamp69.local','lamp67.local','lamp68.local','lamp66.local','kueche.local']


def getOffset(pos, offset):
    return (pos+offset)%len(lamps)

def getRandom():
    return int(random.random()*255)

def getMean(r,g,b):
    return (r+g+b)/255

def isNear(r1,g1,b1,r2,g2,b2):
    return abs(getMean(r1,g1,b1) - getMean(r2,g2,b2)) < 50

t = float(sys.argv[1])

lamps = map( lambda x: libml.Moodlamp(x), lamps)
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
