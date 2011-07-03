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

#hid.connect()
hid.listen(HIDCallback())

#d = uberbus.dispatcher.Dispatcher()
#d.addNode(hid)
#d.setTimer(5, timer)
#d.checkForever()

hid.setTimer(20, time)
hid.checkForever()
#while True:
    #hid.checkOnce()
#    time.sleep(1)
#    hid.set(1)
