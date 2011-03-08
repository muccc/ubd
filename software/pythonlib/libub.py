import socket

class UBNode:
    def __init__(self, address, port, udp = False):
        self.address = address
        self.port = port
        self.udp = udp

    def connect(self):
        self.openSocket()

    def disconnect(self):
        self.closeSocket()

    def setID(self, id):
        self.openMgtSocket()
        ret = self.sendMgtCommand('s%s\x00'%id)
        self.closeMgtSocket()
        return ret
    
    def readResponse(self, s):
        try:
            while True:
                rc = s.recv(1)
                if rc == 'A':
                    return True
                elif rc == 'N':
                    return False
                elif rc == 'C':
                    rc = s.makefile().readline()[:-1]
                    return rc
                elif rc == 'B':
                    count = ord(s.recv(1))
                    rc = ''
                    while len(rc) < count:
                        rc += s.recv(count-len(rc))
                    return rc
                else:
                    print "unknown error while reading response"
                    return ''
        except socket.timeout:
            print "timeout while reading response"
            return False

    def sendCommand(self, command):
        if self.udp:
            self.socket.sendto("%s"%(command),(self.address,self.port))
            return True
        try:
            self.socket.send("B%c%s"%(len(command),command))
            while True:
                response = self.readResponse(self.socket)
                if response == True:
                    return True
                elif response == False:
                    print 'error while sending', list(command)
                    return False
                elif response == '':
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
                    print "unknown error while sending mgt command", \
                            list(command)
                    return False
        except socket.timeout:
            print "timeout while sending mgt command", list(command)
            return False

    def openMgtSocket(self):
        self.mgtsocket = socket.socket(socket.AF_INET6, socket.SOCK_STREAM)
        self.mgtsocket.settimeout(20)
        self.mgtsocket.connect((self.address,2324))       

    def closeMgtSocket(self):
        self.mgtsocket.close();

    def openSocket(self):
        if self.udp:
            self.socket = socket.socket(socket.AF_INET6, socket.SOCK_DGRAM)
        else:
            self.socket = socket.socket(socket.AF_INET6, socket.SOCK_STREAM)
            self.socket.connect((self.address,self.port))
            self.socket.settimeout(20)
    def closeSocket(self):
        self.socket.close();

    def listen(self):
        self.socket.send('L');

