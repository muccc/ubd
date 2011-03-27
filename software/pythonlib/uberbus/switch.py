import socket
import ubnode

class Switch(ubnode.UBNode):
    def __init__(self, address):
        ubnode.UBNode.__init__(self,address,2310)

    def receiveStatus(self):
        while True:
            rc = self.readResponse(self.socket)
            if rc:
                return rc

    def set(self, pin):
        cmd = "S%c"%(pin+0x30)
        return self.sendCommand(cmd)

    def clear(self, pin):
        cmd = "s%c"%(pin+0x30)
        return self.sendCommand(cmd)


