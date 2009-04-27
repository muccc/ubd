import serialinterface
import netinterface
import sys

from packet import Packet
from sys import argv

if len(argv) > 1:
    if argv[1] == 'net':
        con = netinterface.NetInterface()
    elif argv[1] == 'ser':
        con = serialinterface.SerialInterface("/dev/ttyS1",115200)
    else:
        print "unkown input module"
        sys.exit()
else:
    print "please specify an input module"
    sys.exit()
while 1:
    msg = con.readMessage()
    p = Packet(msg)
    print p
    if p.dest == 255 and p.data[0]=='D':
        print "got discover"
con.writeMessage("hello\r\n")
p = Packet();

print "hello"
