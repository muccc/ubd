import socket
import ubnode

class HID(ubnode.UBNode):
    def __init__(self, address):
        ubnode.UBNode.__init__(self,address,2310)

    def set(self, pin):
        cmd = "S%c"%(pin+0x30)
        return self.sendCommand(cmd)

    def clear(self, pin):
        cmd = "s%c"%(pin+0x30)
        return self.sendCommand(cmd)

    def lcd(self, x, y, text):
        cmd = 'D%c%c%s'%(x+0x30,y+0x30,text)
        return self.sendCommand(cmd)

class HIDCallback:
    def newUnsolicited(self, node, data):
        print "hid callback:", list(data)
        self.onUnsolicited(node, data)

        if len(data) == 2 and data[0] == 'B':
            match = True
            self.onButtonPressed(node, ord(data[1]) - 0x30)
        elif len(data) == 2 and data[0] == 'b':
            match = True
            self.onButtonReleased(node, ord(data[1]) - 0x30)
        elif len(data) == 4 and data[0] == 'A':
            channel = ord(data[1])-0x30
            value = (ord(data[2]) << 8) + ord(data[3]);
            self.onAnalogChanged(node, channel, value)
        else:
            self.onUnmatchedUnsolicited(node, data)

    def onButtonPressed(self, node, button):
        pass
    
    def onButtonReleased(self, node, button):
        pass
    
    def onUnsolicited(self, node, data):
        pass

    def onUnmatchedUnsolicited(self, node, data):
        pass

    def onAnalogChanged(self, node, channel, value):
        print 'analog pass'
        pass 
