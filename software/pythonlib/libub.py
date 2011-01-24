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
        return ret

    def sendCommand(self, command):
        try:
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
        except socket.timeout:
            print "timeout while sending command", list(command)
            return False

    def sendMgtCommand(self, command):
        try:
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
        except socket.timeout:
            print "timeout while sending mgt command", list(command)
            return False

    def openMgtSocket(self):
        self.mgtsocket = socket.socket(socket.AF_INET6, socket.SOCK_STREAM)
        self.mgtsocket.settimeout(5)
        self.mgtsocket.connect((self.address,2324))       
    def closeMgtSocket(self):
        self.mgtsocket.close();

    def openSocket(self):
        self.socket = socket.socket(socket.AF_INET6, socket.SOCK_STREAM)
        self.socket.settimeout(5)
        self.socket.connect((self.address,2323))
    def closeSocket(self):
        self.socket.close();

