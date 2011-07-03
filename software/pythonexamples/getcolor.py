#!/usr/bin/python
import libml
import time
import sys

lamp = sys.argv[1]
a = libml.Moodlamp(lamp)

a.connect()
print list(a.getcolor())

