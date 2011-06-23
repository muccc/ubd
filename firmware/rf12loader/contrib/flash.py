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
        if reply[0] == 'P':
            return reply[1:]
        
def expect(data, timeout):
    while 1:
        reply = getReply()
        if reply == data:
            return True
        if reply == False:
            timeout -= 1
            if timeout == 0:
                return False

def send(data):
    serial.writeMessage(data)
    return expect(data[0],1)

def sendPage(pagedata):
    pos = 0
    while 1:
        chunk = pagedata[(pos*32):(pos+1)*32]
        if len(chunk) == 0:
            break
        while not send("D"+chr(pos)+chunk):
            pass
        pos+=1

def reset():
    serial.writeMessage("B\x00\x00\x00\x00\x00\x01\x00\x01r")

def openFile():
    try:
        return open(options.file)
    except IOError:
        print "%s: unable to open file." % (options.file)
        sys.exit(1)

def writeFlash():
    handle = openFile()
    chunkno = 0
    print time.time()
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
    print time.time()
def readFlash():
    handle = openFile()

def writeEeprom():
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

def boot():
    send('G')

def selectDevice():
    while( not send("S%s"%options.name) ):
        print "retry"
    print "device selected"
 
def parse():
    parser = optparse.OptionParser()
    parser.add_option("-m", "--memory", dest = "memtype",
                    default = 'flash',
                    help = "eeprom or flash. Default: flash")
    parser.add_option("-d", "--dev", dest = "serial",
                    default = '/dev/ttyUSB0',
                    help = "serial device. Default: /dev/ttyUSB0")
    parser.add_option("-n", "--name", dest = "name",
                    default = 'newnode,local',
                    help = "name of the device. Default: newnode,local")
    parser.add_option("-a", "--action", dest = "action",
                    default = 'write',
                    help = "write or read. Default: write")
    parser.add_option("-f", "--file", dest = "file",
                    default = 'main.bin',
                    help = "binary file to read or write. Default: main.bin")
    parser.add_option("-s", "--pagesize", dest = "pagesize",
                    default = 256,
                    help = "pagesize of the device. Default: 256")
    parser.add_option("-b", "--baudrate", dest = "baudrate",
                    default = 115200,
                    help = "baudrate of the rf bridge. Default: 115200")
    parser.add_option("-c", "--chunksize", dest = "chunksize",
                    default = 32,
                    help = "size of a single chunk. Default: 32")

    (options, args) = parser.parse_args()
    options.pagesize = int(options.pagesize)
    options.baudrate = int(options.baudrate)
    options.chunksize = int(options.chunksize)
    return options

options = parse()
serial = serialinterface.SerialInterface(options.serial, options.baudrate, 1);
reset()
selectDevice()

if options.memtype == 'flash' and options.action == 'write':
    writeFlash()
if options.memtype == 'flash' and options.action == 'read':
    readFlash()
if options.memtype == 'eeprom' and options.action == 'write':
    writeEeprom()
if options.action == 'boot':
    boot()

