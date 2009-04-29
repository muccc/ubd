import serialinterface
import netinterface
import sys

from packet import Packet
from sys import argv

if len(argv) > 1:
    if argv[1] == 'net':
        con = netinterface.NetInterface()
    elif argv[1] == 'ser':
        con = serialinterface.SerialInterface("/dev/ttyUSB0",115200)
    else:
        print "unkown input module"
        sys.exit()
else:
    print "please specify an input module"
    sys.exit()
#name = ""
a = Packet("")
a.src = 1
a.dest = 255
a.flags = 0
a.seq = 0
a.len = 2
a.data = 'Mr'
con.writeMessage(a.getMessage())

while 1:
    msg = con.readMessage()
    p = Packet(msg)
    if not p.valid:
        continue
    print p
    if p.data[0:2]=='MD':
        print "got discover from "+"".join(p.data[2:])
        a = Packet("");
        a.src = 1
        a.dest = 255
        a.flags = 0
        a.seq = 0
        a.len = p.len+2
        a.data = 'MS'+chr(10)+chr(1)+p.data[2:]
        con.writeMessage(a.getMessage())
    if p.data[0:2]=='MI':
        print "got identify from "+"".join(p.data[2:])
        a = Packet("");
        a.src = 1
        a.dest = p.src
        a.flags = 0
        a.seq = 0
        a.len = p.len+1
        a.data = 'MO'
        con.writeMessage(a.getMessage())
        while con.readMessage() != 'S':
            pass
        name = p.data[2:]
        if name == "newid.local":
            a = Packet("");
            a.src = 1
            a.dest = p.src
            a.flags = 0
            a.seq = 0
            a.len = 2+len(argv[2])+1
            a.data = 'Ms'+argv[2]+chr(0)
            con.writeMessage(a.getMessage())
            while con.readMessage() != 'S':
                pass

    if p.data[0:2]=='MA':
        print "got alive from "+name
        """a = Packet("");
        a.src = 1
        a.dest = p.src
        a.flags = 0
        a.seq = 0
        a.len = p.len+1
        a.data = 'Mg'
        con.writeMessage(a.getMessage())"""

