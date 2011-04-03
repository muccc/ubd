#!/usr/bin/python
import uberbus.moodlamp
import uberbus.hid
import time
import sys

#lamps = ['wipptischlampen.local', 'wipplampelampen.local', 'wipplampen.local']
#lamps = ['kuechenzeile.local', 'kuechelampen.local']

def hsvToRGB(h, s, v):
    """Convert HSV color space to RGB color space
    
    @param h: Hue
    @param s: Saturation
    @param v: Value
    return (r, g, b)  
    """
    import math
    hi = math.floor(h / 60.0) % 6
    f =  (h / 60.0) - math.floor(h / 60.0)
    p = v * (1.0 - s)
    q = v * (1.0 - (f*s))
    t = v * (1.0 - ((1.0 - f) * s))
    return {
        0: (v, t, p),
        1: (q, v, p),
        2: (p, v, t),
        3: (p, q, v),
        4: (t, p, v),
        5: (v, p, q),
    }[hi]

hidname = sys.argv[1]
lamps = sys.argv[2:]

hid = uberbus.hid.HID(hidname)
hid.lcd(0,1,'White Mode Group')
mode = 1
s = 1
v = 1
h = 0

r = g = b = 0
lamp = 0
hid.lcd(0,0,lamps[lamp])
class HIDCallback(uberbus.hid.HIDCallback):
    def onButtonPressed(self, node, button):
        global lamp, lamps, mode
        if button == 0:
            hid.clear(7-lamp)
            lamp += 1
            if lamp == len(lamps):
                lamp = 0
            hid.lcd(0,0,lamps[lamp])
        elif button == 1:
            if mode == 1:
                mode = 2
                hid.lcd(0,1,'Off   Mode Group')
            elif mode == 2:
                mode = 1
                hid.lcd(0,1,'White Mode Group')
        elif button == 6:
            a = uberbus.moodlamp.Moodlamp(lamps[lamp],True)
            print "connecting to", lamps[lamp]
            a.connect()
            if mode == 1:
                a.setcolor(255,255,255)
            elif mode == 2:
                a.setcolor(0,0,0)
                
    def onAnalogChanged(self, node, channel, value):
        global h,s,v,r,g,b,lamp,lamps
        #value = (ord(rc[2]) << 8) + ord(rc[3]);
        if channel == 4:
            h = value * (360./1024.)
            print 'h=',h
        elif channel == 5:
            v = 1. - value / 1024.
            print 'v=',v
        elif channel == 6:
            s = value / 1024.
            print 'v=',v
        (r,g,b) = hsvToRGB(h,s,v)
        a = uberbus.moodlamp.Moodlamp(lamps[lamp],True)
        print "connecting to", lamps[lamp]
        a.connect()
        a.timedfade(int(r*255),int(g*255),int(b*255),.5)


hid.listen(HIDCallback())
hid.checkForever()

