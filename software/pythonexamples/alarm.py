#!/usr/bin/python
import uberbus.digitalinput
import uberbus.digitaloutput

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

do.connect(True)
di.connect(True)
di.listen(DigitalInputCallback())
di.checkForever()
