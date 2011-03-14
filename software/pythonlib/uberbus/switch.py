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
