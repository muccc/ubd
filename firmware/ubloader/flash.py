
#! /usr/bin/python
import sys
import serialinterface
import optparse
import time

def getReply():
    while 1:
        reply = serial.readMessage()
        if reply == False:
            print "device not responding"
            return False
        #if reply[0] == 'P':
        return reply
        
def expect(data, timeout):
    while 1:
        reply = getReply()
        #print "got", reply
        if reply == False:
            timeout -= 1
            if timeout == 0:
                return False
        if reply[0:1] == data:
            return reply

def send(data, ret=False):
    serial.writeMessage(data)
    d = expect(data[0],1)
    if ret == False:
        return d
    return d[1:]


def sendPage(pagedata):
    pos = 0
    while 1:
        chunk = pagedata[(pos*32):(pos+1)*32]
        if len(chunk) == 0:
            break
        while not send("P"+chr(pos)+chunk):
            pass
        pos+=1

def reset():
    #cmd = "".join(['\x01', '\xff', '\x18', '\x04', 'r', '\xa6', '\xc3'])
    #serial.write("\\0\x00r\\1")
    #serial.write("\\1\x01\xFF\x18\x00\x01r\xa6\xc3\\2")
    serial.write("\\1\x01\xFF\x18\x00\x01r\xb5\x07\\2")
    #serial.write(cmd)
def openFile():
    try:
        return open(options.file)
    except IOError:
        print "%s: unable to open file." % (options.file)
        sys.exit(1)

def writeFlash():
    handle = openFile()
    chunkno = 0
    while 1:
        chunk = handle.read (options.pagesize)
        chunklen = len (chunk)
        if chunklen == 0:
            break;
        while len (chunk) < options.pagesize:
            chunk = chunk + "\377"
        print "%02x (%02x): " % (chunkno, chunklen)

        sendPage(chunk)
        while not send('F'+chr(chunkno)):
            pass
        chunkno+=1

def readFlash():
    handle = openFile()

def readEeprom():
    #l = options.len
    adr = int(options.adr)
    #if l == -1:
    #    l = len(options.name)
    l = 64
    print list(send("R%c%c%c"%(adr>>8,adr&0xFF,l),True))

def writeEeprom():
    adr = int(options.adr)
    data = options.data.decode('string_escape')
    send("E%c%c%s"%(adr>>8,adr&0xFF,data))
    """
handle = openFile()
    chunkno = 0
    while 1:
        chunk = handle.read (options.chunksize)
        chunklen = len (chunk)
        if chunklen == 0:
            break;
        while len (chunk) < options.chunksize:
            chunk = chunk + "\377"
        print "%02x (%02x): " % (chunkno, chunklen)

        while not send('E'+chr(chunkno)+chunk):
            pass
        chunkno+=1
    """
    print "blubb"
def boot():
    send('G')

def selectDevice():
    #while( not send("SF\x00\x00%c%s"%(len(options.name),options.name)) ):
    #l = options.len
    start = int(options.start)
    name = options.name.decode('string_escape')
    #if l == -1:
    #    l = len(options.name)

    serial.writeMessage("BUBBOOTLOADER")
    while( not send("SE%c%c%s"%(start>>8,start&0xFF,name)) ):
        print "retry"
        serial.writeMessage("BUBBOOTLOADER")
    print "device selected"
 
def parse():
    parser = optparse.OptionParser()
    parser.add_option("-m", "--memory", dest = "memtype",
                    default = 'flash',
                    help = "eeprom or flash. Default: flash")
    parser.add_option("-D", "--dev", dest = "serial",
                    default = '/dev/ttyUSB0',
                    help = "serial device. Default: /dev/ttyUSB0")
    parser.add_option("-n", "--name", dest = "name",
                    default = 'newlamp.local',
                    help = "name of the device. Default: newlamp.local")
    parser.add_option("-A", "--action", dest = "action",
                    default = 'select',
                    help = "slect, write, read or boot. Default: select")
    parser.add_option("-f", "--file", dest = "file",
                    default = 'main.bin',
                    help = "binary file to read or write")
    parser.add_option("-p", "--pagesize", dest = "pagesize",
                    default = 256,
                    help = "pagesize of the device")
    parser.add_option("-b", "--baudrate", dest = "baudrate",
                    default = 115200,
                    help = "baudrate of the rf bridge")
    parser.add_option("-c", "--chunksize", dest = "chunksize",
                    default = 32,
                    help = "size of a single chunk")
    parser.add_option("-s", "--start",
                    dest = "start",
                    default = 2048-51,
                    help = "start of name in device. Default: E2END-50")
    parser.add_option("-a", "--address",
                    dest = "adr",
                    default = 2048-51,
                    help = "address of eeprom actions. Default: E2END-50")
    parser.add_option("-d", "--data",
                    dest = "data",
                    default = "",
                    help = "data to program to eeprom")

#    parser.add_option("-d", "--data",
#                    dest = "data",
#                    default = "",
#                    help = "data to write to eeprom")

    (options, args) = parser.parse_args()
    options.pagesize = int(options.pagesize)
    options.baudrate = int(options.baudrate)
    options.chunksize = int(options.chunksize)
    return options

options = parse()
serial = serialinterface.SerialInterface(options.serial, options.baudrate, 1);

"""
t = 0.001
n = 300
while 1:
    serial.write(    "\x5c\x30\x00\x43\xff\x12\x12\x5c\x31")
    time.sleep(t)
    for i in range(1,n):
        serial.write("\x5c\x30\x32\x43\x12\x12\x12\x5c\x31")
        time.sleep(t)
    print "."
    time.sleep(t)
    serial.write(    "\x5c\x30\x00\x43\x12\x12\x12\x5c\x31")
    for i in range(1,n):
        serial.write("\x5c\x30\x32\x43\x12\x12\x12\x5c\x31")
        time.sleep(t)
    print ","
#    time.sleep(0.005)
"""

if options.action == 'select':
   reset()
   selectDevice()
if options.memtype == 'flash' and options.action == 'write':
    writeFlash()
if options.memtype == 'flash' and options.action == 'read':
    readFlash()
if options.memtype == 'eeprom' and options.action == 'read':
    readEeprom()
if options.memtype == 'eeprom' and options.action == 'write':
    writeEeprom()
if options.action == 'boot':
    boot()

