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
        print 'writing %s' % list(enc)
        self.ser.write(enc)

    def readMessage(self):
        data = ""
        escaped = False
        stop = False
        start = False
        inframe = False

        while True:
            c = self.ser.read(1)
            if len(c) == 0:             #A timout occured
                print 'TIMEOUT'
                return False
            #print "c=", c
        #    continue
            if escaped:
                if c == '1':
                    start = True
                    inframe = True
                elif c == '2':
                    stop = True
                    inframe = False
                elif c == '\\':
                    d = '\\'
                escaped = False
            elif c == '\\':
                escaped = 1
            else:
                d = c
                
            if start and inframe:
                start = False
            elif stop:
                if data[0] == 'D':
                    message = '%f %s'%(time.time(), data[2:])
                    print 'serial debug message:',data
                    #print message
                    data = ""
                    stop = False
                else:
                    #print 'received message: len=%d data=%s'%(len(data),data)
                    #print 'received message:',list(data)
                    return data
            elif escaped == False and inframe:
                data += str(d)

