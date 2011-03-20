#!/usr/bin/python

import gtk
import sys
import dbus, gobject, avahi
from dbus.mainloop.glib import DBusGMainLoop
import uberbus.moodlamp

TYPE = "_moodlamp._udp"
#TODO: Text input for fadetime
t = 0.5
icon = "/usr/share/ub-colorgui/ml_icon.png"
#TODO: Autodetect lamps using avahi
#lamps = ["alle.local", "moon.local", "spot.local", "oben.local", "unten.local"]

class UBColorGui(object):
    def __init__(self, loop):
        window = gtk.Window()
        vbox1 = gtk.VBox()
        hbox1 = gtk.HBox()
        window.add(vbox1)
        window.connect("delete_event", gtk.main_quit)
        window.set_border_width(5)
        window.set_icon_from_file(icon)
        color = gtk.ColorSelection()
        color.connect("color_changed",self.new_color)
        self.combobox = gtk.combo_box_new_text()
        self.combobox.set_border_width(5)
        label = gtk.Label("Selected Lamp:")
        separator = gtk.HSeparator()
        vbox1.pack_start(hbox1)
        hbox1.pack_start(label, False, False, 1)
        hbox1.pack_start(self.combobox, True, True, 2)
        vbox1.pack_start(separator)
        vbox1.pack_start(color,True,True,2)
        hbox1.set_border_width(5)
        window.show_all()

        bus = dbus.SystemBus(mainloop=loop)
        self.server = dbus.Interface(bus.get_object(avahi.DBUS_NAME, '/'), 'org.freedesktop.Avahi.Server')
        sbrowser = dbus.Interface(bus.get_object(avahi.DBUS_NAME,
            self.server.ServiceBrowserNew(avahi.IF_UNSPEC,
                avahi.PROTO_UNSPEC, TYPE, 'local', dbus.UInt32(0))),
            avahi.DBUS_INTERFACE_SERVICE_BROWSER)
        sbrowser.connect_to_signal("ItemNew", self.mlfound)

    def mlfound(self, interface, protocol, name, stype, domain, flags):
        print "Found service '%s' type '%s' domain '%s' " % (name, stype, domain)

        self.server.ResolveService(interface, protocol, name, stype,
            domain, avahi.PROTO_UNSPEC, dbus.UInt32(0),
            reply_handler=self.service_resolved, error_handler=self.print_error)

    def service_resolved(self, *args):
        print 'service resolved'
        print 'name:', args[2]
        self.combobox.append_text("%s.local" % args[2])
#        print 'address:', args[7]
#        print 'port:', args[8]

    def print_error(self, *args):
        print 'error_handler'
        print args[0]

    def new_color(self, color):
        model = self.combobox.get_model()
        index = self.combobox.get_active()
        if index:
            lamp = model[index][0]
            print "Active lamp: %s" % lamp
            s = uberbus.moodlamp.Moodlamp(lamp, True)
            c = color.get_current_color()
            r = c.red/256;
            g = c.green/256;
            b = c.blue/256;
            s.connect()
            s.timedfade(r,g,b,t)
            print "Setting %s to %s%s%s" % (lamp, hex(r)[2:], hex(g)[2:], hex(b)[2:])
            s.disconnect()

def main():
    loop = DBusGMainLoop()
    bcb = UBColorGui(loop)
    gtk.main()

if __name__== "__main__":
    main()
