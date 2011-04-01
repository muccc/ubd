#!/usr/bin/python
import gtk
import sys
import uberbus.moodlamp

t = 1
lamp = sys.argv[1]
s = uberbus.moodlamp.Moodlamp(lamp, True)

def new_color(color):
    c = color.get_current_color()
    r = c.red/256;
    g = c.green/256;
    b = c.blue/256;
    s.connect();
    s.timedfade(r,g,b,t)

window = gtk.Window()
window.connect("delete_event", gtk.main_quit)
window.set_border_width(10)
color = gtk.ColorSelection()
color.connect("color_changed",new_color)
window.add(color)
window.show_all()
gtk.main() 
