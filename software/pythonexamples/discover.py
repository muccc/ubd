#!/usr/bin/python
import uberbus.moodlamp
import gobject

class Resolver(uberbus.moodlamp.MoodlampResolver):
    def newNode(self, node, address, multicast):
        print node, address, multicast
        if multicast == True:
            m = uberbus.moodlamp.Moodlamp(address, udp=True)
            m.connect()
            m.setcolor(0,0,255)

Resolver(udp=True)
gobject.MainLoop().run()
