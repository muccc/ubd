import serial
import string
import sys
import time

class SerialInterface:
    def  __init__ ( self, path2device, baudrate, timeout=0):
        self.ser = serial.Serial(path2device, baudrate)
        self.ser.flushInput()
        self.ser.flushOutput()
        if timeout:
            self.ser.setTimeout(timeout)

    def writeMessage(self,message):
        enc = "\\1" + message.replace('\\','\\\\') + "\\2";
        #print "writing %s"% enc
        self.ser.write(enc)

    def readMessage(self):
        data = ""
        escaped = False
        stop = False
        start = False

        while True:
            c = self.ser.read(1)
            if len(c) == 0:             #A timout occured
                print 'TIMEOUT'
                return False
        #    print "c=", c
        #    continue
            if escaped:
                if c == '1':
                    start = True
                elif c == '2':
                    stop = True
                elif c == '\\':
                    d = '\\'
                escaped = False
            elif c == '\\':
                escaped = 1
            else:
                d = c
                
            if start:
                start = False
            elif stop:
                if data[0] == 'D':
                    #print 'serial debug message: %d %s'%(len(data), data)
                    data = ""
                    stop = False
                else:
                    #print 'received message: len=%d data=%s'%(len(data),data)
                    return data
            elif escaped == False:
                data += str(d)
