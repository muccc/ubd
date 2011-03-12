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
play = False
pygame.mixer.init()
music.load('siren.wav')

def setcolors():
    global state
    global play
    while 1:
        if state == 'b0':
            a.timedfade(0,0,255,1)
            music.stop()
            play = False
            state = 'b1'
        elif state == 'b1':
            time.sleep(.1)
        elif state == 'r1':
            a.timedfade(255,0,0,.3)
            if not play:
            	music.play(-1)
                play = True
            state = 'r2'
            time.sleep(.3);
        elif state == 'r2':
            a.timedfade(0,0,0,.3)
            state = 'r1'
            time.sleep(.3)

thread.start_new_thread(setcolors,())

while True:
    #time.sleep(1)
    #continue
    rc = s.receiveStatus()
    if rc == 'b0':
        state = 'b0'
        #a.timedfade(0,0,255,1)
    elif rc == 'B0':
        state = 'r1'
        #a.timedfade(255,0,0,1)

