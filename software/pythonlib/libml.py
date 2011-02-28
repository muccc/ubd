import socket
import libub

class Moodlamp(libub.UBNode):
    def __init__(self, address, udp = False):
        libub.UBNode.__init__(self,address,2323,udp)

    def setcolor(self, r, g, b):
        cmd = "C%c%c%c"%(r,g,b)
        return self.sendCommand(cmd)
    
    def fade(self, r, g, b, speed):
        h = int(speed)>>8
        l = int(speed)&0xFF
        cmd = "F%c%c%c%c%c"%(r,g,b,h,l);
        return self.sendCommand(cmd)

    def timedfade(self, r, g, b, time, allchannelsequal=True):
        time = time * 1000
        h = int(time)>>8
        l = int(time)&0xFF
        if allchannelsequal:
            fadecmd = 'T'
        else:
            fadecmd = 'M'
        cmd = "%c%c%c%c%c%c"%(fadecmd,r,g,b,h,l);
        return self.sendCommand(cmd)

    def setBrightness(self, brightness):
        cmd = "D%c"%brightness;
        return self.sendCommand(cmd)
    def getVersion(self):
        cmd = 'V'
        if self.sendCommand(cmd):
            return self.readResponse(self.socket)
        else:
            return False
