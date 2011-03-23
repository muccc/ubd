#!/usr/bin/python
import uberbus.moodlamp
import uberbus.switch
import time
import sys
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

lamp = sys.argv[1]
switch = sys.argv[2]

a = uberbus.moodlamp.Moodlamp(lamp,True)
hid = uberbus.switch.Switch(switch)

a.connect()
hid.connect()
hid.listen()

s = 1
v = 1
h = 0

r = g = b = 0
while True:
    rc = hid.receiveStatus()
    if rc[0] == 'A':
        channel = rc[1]
        value = (ord(rc[2]) << 8) + ord(rc[3]);
        if channel == '4':
            h = value * (360./1024.)
            print 'h=',h
        elif channel == '5':
            v = value / 1024.
            print 'v=',v
    (r,g,b) = hsvToRGB(h,s,v)
    print 'r=',r
    print 'g=',g
    print 'b=',b
    a.timedfade(int(r*255),int(g*255),int(b*255),.5)

