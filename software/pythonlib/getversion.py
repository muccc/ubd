#!/usr/bin/python
import libml
import sys
a = libml.Moodlamp(sys.argv[1])
a.connect()
print a.getVersion()
