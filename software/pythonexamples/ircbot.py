#!/usr/bin/python
import irclib
import uberbus.moodlamp
import uberbus.switch
import time

#irc = irclib.IRC()
#server = irc.server()
#server.connect("irc.ray.net", 6667, "uberbus")
#server.privmsg("s", "Hi there!")
#irc.process_forever()


class uberbusbot(irclib.SimpleIRCClient):
    def __init__(self):
        irclib.SimpleIRCClient.__init__(self)
        self.connect("irc.ray.net", 6667, "uberbus")

    def on_privmsg(self, connection, event):
        self.setcolor(event.source(), event.arguments()[0])
    def on_pubmsg(self, connection, event):
        self.setcolor(event.target(), event.arguments()[0])

    def setcolor(self, target, argument):
        print target
        t = argument.split(' ')
        try:
            if t[0] == '!set':
                print t
                print "got !set"
                if len(t) < 2:
                    return
                lamp = t[1] + ".local"
                r = b = g = 0
                if len(t) > 2:
                    r = int(t[2])
                if len(t) > 3:
                    g = int(t[3])
                if len(t) > 4:
                    b = int(t[4])
                l = uberbus.moodlamp.Moodlamp(lamp, True)
                l.connect()
                l.setcolor(r,g,b)
                self.connection.privmsg(target,'core meltdown!')
            elif t[0] == '!get':
                print "got !get"
                if len(t) < 2:
                    return
                lamp = t[1] + ".local"
                l = uberbus.moodlamp.Moodlamp(lamp)
                l.connect()
                r = l.getcolor()
                self.connection.privmsg(target,[ord(i) for i in r])
            elif t[0] == "!alarm":
                switch = uberbus.switch.Switch("laboralarm.local")
                switch.set("alarm")
                self.connection.privmsg(target,'alarmiere das Labor!')
                time.sleep(10)
                switch.clear("alarm")
        except Exception as e:
            self.connection.privmsg(target,e)
            print e
    def on_welcome(self, connection, event):
        self.join("#uberbus")
        self.join("#santa")
        self.join("#ccc")
    def join(self, channel):
        print "joining", channel
        self.connection.join(channel)
bus = uberbusbot()
bus.start()
