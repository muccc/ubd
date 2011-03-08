import socket
import libub

class Switch(libub.UBNode):
    def __init__(self, address):
        libub.UBNode.__init__(self,address,2310)

    def receiveStatus(self):
        while True:
            rc = self.readResponse(self.socket)
            if rc:
                return rc
