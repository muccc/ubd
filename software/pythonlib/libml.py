import socket
import libub

class Moodlamp(libub.UBNode):

    def setcolor(self, r, g, b):
        cmd = "C%c%c%c"%(r,g,b)
        self.sendCommand(cmd)
    
    def fade(self, r, g, b, speed):
        h = int(speed)>>8
        l = int(speed)&0xFF
        cmd = "F%c%c%c%c%c"%(r, g, b, h, l);
        self.sendCommand(cmd)

    def fadems(self, r, g, b, time):
        h = int(time)>>8
        l = int(time)&0xFF
        cmd = "M%c%c%c%c%c"%(r, g, b, h, l);
        self.sendCommand(cmd)
    def setBrightness(self, brightness):
        cmd = "D%c"%brightness;
        self.sendCommand(cmd)
