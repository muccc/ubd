#!/usr/bin/python
import libml
import libswitch
import time
import sys
import threading
import thread
import pygame
from pygame.mixer import music


lamp = sys.argv[1]
switch = sys.argv[2]

a = libml.Moodlamp(lamp,True)
s = libswitch.Switch(switch)

a.connect()
s.connect()
s.listen()
state = 'b'
pygame.mixer.init()
music.load('siren.wav')

def setcolors():
    global state
    while 1:
        if state == 'b':
            a.timedfade(0,0,255,1)
            time.sleep(1);
        elif state == 'r1':
            a.timedfade(255,0,0,.3)
            state = 'r2'
            time.sleep(.3);
        elif state == 'r2':
            a.timedfade(0,0,0,.3)
            state = 'r1'
            time.sleep(.3);

thread.start_new_thread(setcolors,())

while True:
    rc = s.receiveStatus()
    if rc == 'b0':
        state = 'b'
        music.stop()
        #a.timedfade(0,0,255,1)
    elif rc == 'B0':
        state = 'r1'
        music.play(-1)
        #a.timedfade(255,0,0,1)

