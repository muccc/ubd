import socket

class UBNode:
    """
    A base class for uberbus nodes.

    This class provides basic TCP and UDP support
    to communicate with uberbus nodes.

    """
    def __init__(self, address, port, udp = False):
        """
        Create a new uberbus node object.
        
        Parameters:
        address -- The hostname or ip address of the node
        port    -- The port to connect on the node. This determines the service to be used.
        udp     -- If set to True commands will be sent with UDP packets.

        """
        self.address = address
        self.port = port
        self.udp = udp
        self.socket = False

    def connect(self):
        """ Open a connection to the node.

        Return value: None

        Exceptions: Throws exceptions if the connection failed.
        
        """
        self.openSocket()

    def disconnect(self):
        """ Close the connection to the node.

        Return value: None
        
        """
        self.closeSocket()

    def setID(self, id):
        """ Set the ID of a node

        Opens a connection to the management service of the node
        and tries to set its ID. See the uberbus
        documentation on the format of an ID.

        Return value: True if the command was successfull.

        Exceptions: Throws exceptions if no connection
                    can be opened to the node.

        """
        self.openMgtSocket()
        ret = self.sendMgtCommand('s%s\x00'%id)
        self.closeMgtSocket()
        return ret

    def readResponse(self, s):
        """
        Read the response of the node.

        After sending a command with sendCommand() or listening to
        the node with listen(), responses from the node can be
        received.

        Return values:  False if a timeout occoured
                        The received response if a respnse from the
                        node arrived.

        """
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
        """
        Send a command to the node.

        Before using this operation the node has to be connected
        with connect()
        The command will be send using the binary uberbus protocol.

        Return value: True if the command was sent successfully.

        """
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
        self.mgtsocket.close()

    def openSocket(self):
        if self.udp:
            self.socket = socket.socket(socket.AF_INET6, socket.SOCK_DGRAM)
        else:
            self.socket = socket.socket(socket.AF_INET6, socket.SOCK_STREAM)
            self.socket.connect((self.address,self.port))
            self.socket.settimeout(20)
    def closeSocket(self):
        if not self.udp and self.socket:
            self.socket.close()
            self.socket = False

    def listen(self):
        """
        Listen to messages from the node.

        Nodes can send messages which are not in response to commands.
        To receive theses messages listen() has to be called.
        Messages can then be read using readRespnse()

        Returns noting.

        """
        self.socket.send('L')

