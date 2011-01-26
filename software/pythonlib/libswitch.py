import socket
import libub

class Switch(libub.UBNode):
    def receiveStatus(self):
        while True:
            rc = self.readResponse(self.socket)
            if rc:
                return rc
