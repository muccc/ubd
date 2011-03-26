import threading
import serial
import string
import curses
import sys
import time
import math
import random

ser =  serial.Serial(sys.argv[1], 115200)
t0 = time.time()
t = time.time()

data = []
#ser.write(aus)
escaped = False
stop = False
start = False
inframe = False
discover = False
query = False
while True:
    c = ser.read(1)
    #print "c=",ord(c),hex(ord(c))
    #t = time.time()
    #print "%f"%t, list(c), hex(ord(c))
    #continue
    if escaped:
        escaped = False
        if c == '1':
            start = True
            inframe = True
            #print "start"
        elif c == '2':
            stop = True
            inframe = False
        elif c == '\\':
            d = '\\'
        elif c == '3':
            print "%f: discover"%(t)
            continue
        elif c == '4':
            start = True
            query = True
            continue
        else:
            print "out of order escape"
    elif c == '\\':
        t = time.time()
        escaped = 1
    else:
        d = c
        
    if start and inframe:
        start = False
    elif start and query:
        print "%f: query for %d"%(t,ord(c))
        query = False
        start  = False
    elif stop:
        print "%f"%(t), len(data) ,"".join(data)
        data = []
        stop = False
    elif escaped == False and inframe:
        data.append(d)
    elif escaped == False:
        print "%f: c= %d"%(t,ord(c))

