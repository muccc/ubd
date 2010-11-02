import socket
#bus = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)

class UBNode:
    def __init__(self, address):
        self.address = address

    def connect(self):
        return True

    def setID(self, id):
        self.openSocket()
        ret = self.sendCommand('s%s\x00'%id)
        self.closeSocket()
        print "setID returns", ret

    def sendCommand(self, command):
        self.socket.send("B%c%s"%(len(command),command))
        while True:
            rc = self.socket.recv(1)
            if rc == 'A':
                return True
            elif rc == 'N':
                return False

    def openSocket(self):
        self.socket = socket.create_connection(
                (self.address,2324))
    def closeSocket(self):
        self.socket.close();
