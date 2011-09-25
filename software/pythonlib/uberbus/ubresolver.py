import dbus, gobject, avahi
from dbus import DBusException
from dbus.mainloop.glib import DBusGMainLoop
import os

class UBResolver: 
    def __init__(self, ubclass, udp = False):
        self.loop = DBusGMainLoop()
        self.bus = dbus.SystemBus(mainloop=self.loop)

        self.server = dbus.Interface( self.bus.get_object(avahi.DBUS_NAME, '/'),
                'org.freedesktop.Avahi.Server')
        if udp:
            self.avahitype = ubclass.udptype
        else:
            self.avahitype = ubclass.tcptype

        self.sbrowser = dbus.Interface(self.bus.get_object(avahi.DBUS_NAME,
                self.server.ServiceBrowserNew(avahi.IF_UNSPEC,
                    avahi.PROTO_UNSPEC, self.avahitype, 'local', dbus.UInt32(0))),
                avahi.DBUS_INTERFACE_SERVICE_BROWSER)
        
        self.nodes = []

        self.sbrowser.connect_to_signal("ItemNew", self.newhandler)
        self.sbrowser.connect_to_signal("ItemRemoved", self.removehandler)

    def service_resolved(self, *args):
        #print 'service resolved'
        #print 'name:', args[2]
        #print 'address:', args[7]
        #print 'port:', args[8]
        
        address = args[7]
        name = args[2]
        domain = 'local'

        node = name + '.' + domain
        #print 'node',node

        if node not in self.nodes:
            self.nodes.append(node)
            multicast = False
            if address[0:2] == 'ff':
                multicast = True
            self.newNode(node, address, multicast)

    def print_error(self, *args):
        print 'error_handler'
        print args[0]
        
    def removehandler(self, interface, protocol, name, stype, domain, flags):
        print "Removed service '%s' type '%s' domain '%s' " % (name, stype, domain)
        print interface, protocol, name, stype, domain, flags
        node = name + '.' + domain
        if node in self.nodes:
            self.nodes.remove(node)
            self.removedNode(node)

    def newhandler(self, interface, protocol, name, stype, domain, flags):
        #print "Found service '%s' type '%s' domain '%s' " % (name, stype, domain)
        #print interface, protocol, name, stype, domain, flags
        self.server.ResolveService(interface, protocol, name, stype, 
            domain, avahi.PROTO_UNSPEC, dbus.UInt32(0), 
            reply_handler=self.service_resolved, error_handler=self.print_error)

    def newNode(self, node, address, multicast):
        pass

    def removedNode(self, node):
        pass

    def clear(self):
        self.nodes = []

