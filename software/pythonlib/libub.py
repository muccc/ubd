import socket
#bus = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)

class UBNode:
    def __init__(self, address):
        self.address = address

    def connect(self):
        self.openSocket()

    def disconnect(self):
        self.closeSocket()

    def setID(self, id):
        self.openMgtSocket()
        ret = self.sendMgtCommand('s%s\x00'%id)
        self.closeMgtSocket()
        print "setID returns", ret

    def sendCommand(self, command):
        self.socket.send("B%c%s"%(len(command),command))
        while True:
            rc = self.socket.recv(1)
            if rc == 'A':
                return True
            elif rc == 'N':
                print "error while sending command", list(command)
                return False
            else:
                print "unknown error while sending command", list(command)
                return False

    def sendMgtCommand(self, command):
        self.mgtsocket.send("B%c%s"%(len(command),command))
        while True:
            rc = self.mgtsocket.recv(1)
            if rc == 'A':
                return True
            elif rc == 'N':
                print "error while sending mgt command", list(command)
                return False
            else:
                print "unknown error while sending mgt command", list(command)
                return False

    def openMgtSocket(self):
        self.mgtsocket = socket.create_connection(
                (self.address,2324))
    def closeMgtSocket(self):
        self.mgtsocket.close();

    def openSocket(self):
        self.socket = socket.create_connection(
                (self.address,2323))
    def closeSocket(self):
        self.socket.close();

