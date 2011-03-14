#!/usr/bin/python
import uberbus.moodlamp
import sys
a = uberbus.moodlamp.Moodlamp(sys.argv[1])
a.connect()
print a.getVersion()
