#!/usr/bin/python
import libml
import time
import sys

lamp = sys.argv[1]
r = int(sys.argv[2])
g = int(sys.argv[3])
b = int(sys.argv[4])
t = float(sys.argv[5])

a = libml.Moodlamp(lamp)
a.connect();

if a.timedfade(r,g,b,t):
    sys.exit(0)
sys.exit(1)

