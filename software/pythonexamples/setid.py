#!/usr/bin/python
import uberbus.moodlamp
import sys
a = uberbus.moodlamp.Moodlamp(sys.argv[1])
a.setID(sys.argv[2])
