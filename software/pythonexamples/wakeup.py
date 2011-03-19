#!/usr/bin/python
import uberbus.moodlamp
import time
import sys
import pygame.mixer as m
import os.path as p

#### WAKEUP SCRIPT ####
# This script fades lamps and/or groups of lamps through a number of steps and
# optionally plays a sound.

#conf
audio = "" # add optional wake up song here
first = {'lamp' : 'foo.local', 'r' : 100, 'g' : 50 , 'b' : 5, 't' : 65} #t in seconds must be equal or less than 65 
second = {'lamp' : 'group.local', 'r' : 245, 'g' : 170 , 'b' : 40, 't' : 65} # r, g, b: 0-255
steps = [first,second]

def fade ( lamp, r, g, b, t ):
  "fade lamp/group to r g b in t"
  a = uberbus.moodlamp.Moodlamp(lamp,True)
  a.connect()

  a.timedfade(r,g,b,t)
  a.disconnect()
  time.sleep(t)
  return

for step in steps:
  fade (step['lamp'],step['r'],step['g'],step['b'],step['t'])
  time.sleep(10)

if audio:
 if p.isfile(audio):
  m.init(44100)
  m.music.load(audio)
  m.music.set_volume(0.6)
  m.music.play()
  while m.music.get_busy():
   time.sleep(1)
sys.exit(0)
