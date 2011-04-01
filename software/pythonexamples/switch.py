#!/usr/bin/python
import uberbus.moodlamp
import uberbus.hid
import time
import sys

lampname = sys.argv[1]
hidname = sys.argv[2]

lamp = uberbus.moodlamp.Moodlamp(lampname,True)
hid = uberbus.hid.HID(hidname)

class HIDCallback(uberbus.hid.HIDCallback):
    def onButtonPressed(self, node, button):
        global lamp
        lamp.timedfade(255,0,0,1)
    def onButtonReleased(self, node, button):
        global lamp
        lamp.timedfade(0,0,255,1)

lamp.connect()
hid.connect()
hid.listen(HIDCallback())
hid.checkForever()
