#!/usr/bin/python
import uberbus.digitalinput
import uberbus.digitaloutput
#import uberbus.dispatcher

import time
import sys

adr = sys.argv[1]
adr2 = sys.argv[2]
di = uberbus.digitalinput.DigitalInput(adr)
do = uberbus.digitaloutput.DigitalOutput(adr2)

class DigitalInputCallback(uberbus.digitalinput.DigitalInputCallback):
    def onInput(self, node, pin, state):
        print "pin",pin,'=',state
        if pin == 'alarm' and state == 0:
            do.set('alarm')
        elif pin == 'alarm' and state == 1:
            do.clear('alarm')
        #hid.abort()
        #d.abort()

def timer():
    print time.time(), "ontimer"

#hid.connect()
do.connect()
di.listen(DigitalInputCallback())

#d = uberbus.dispatcher.Dispatcher()
#d.addNode(hid)
#d.setTimer(5, timer)
#d.checkForever()

#hid.setTimer(20, time)
di.checkForever()
#while True:
    #hid.checkOnce()
#    time.sleep(1)
#    hid.set(1)
