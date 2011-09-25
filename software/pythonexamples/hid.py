#!/usr/bin/python
import uberbus.hid
#import uberbus.dispatcher

import time
import sys

adr = sys.argv[1]
hid = uberbus.hid.HID(adr)
class HIDCallback(uberbus.hid.HIDCallback):
    def onButtonPressed(self, node, button):
        print "button",button,"pressed"
        hid.abort()
        #d.abort()
def timer():
    print time.time(), "ontimer"

hid.connect(True)

hid.listen(HIDCallback())
hid.setTimer(20, time)
hid.checkForever()
