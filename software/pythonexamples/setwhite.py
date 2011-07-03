#!/usr/bin/python
import uberbus.moodlamp
import time
import sys

lamp = sys.argv[1]
a = uberbus.Moodlamp(lamp)
a.connect();
a.setcolor(255,255,255)

