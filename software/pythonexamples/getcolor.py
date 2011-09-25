#!/usr/bin/python
import uberbus.moodlamp
import time
import sys

lamp = sys.argv[1]
a = uberbus.moodlamp.Moodlamp(lamp)

a.connect()
print list(a.getcolor())

