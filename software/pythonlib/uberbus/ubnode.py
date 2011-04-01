import socket
import select
import parser
import Queue
import time
import dispatcher

class UBNode(parser.Parser):
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
        parser.Parser.__init__(self)
        self.messagequeue = Queue.Queue()
        self.unsolicitedqueue = Queue.Queue()
        
        self.address = address
        self.port = port
        self.udp = udp

        self.receivetimeout = 10
        self.keepconnection = True

        self.socket = None
        self.timeout =None
        self.time = None
        self.dispatcher = None
        self.callbacks = []
        self.aborted = False
        self.lasttry = 0

    def setKeepConnection(self, keep):
        self.keepconection = keep

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

    def openSocket(self):
        if self.udp:
            self.socket = socket.socket(socket.AF_INET6, socket.SOCK_DGRAM)
        else:
            if time.time() - self.lasttry >= self.timeout:
                self.socket = socket.socket(socket.AF_INET6, socket.SOCK_STREAM)
                self.socket.connect((self.address,self.port))
                self.socket.settimeout(0)
                self.lasttry = time.time()

    def closeSocket(self):
        if not self.udp and self.socket:
            self.socket.close()
            self.socket = None

    def getSocket(self):
        return self.socket

    def listen(self, callbackobject):
        """
        Listen to messages from the node.

        Nodes can send messages which are not in response to commands.
        To receive theses messages listen() has to be called.
        Messages can then be read using readRespnse()

        Returns noting.

        """
        self.callbacks.append(callbackobject)

    def eof(self):
        #gets called by the dispatcher when the sockets says EOF
        print "EOF on socket"
        self.closeSocket()

    def checkConnection(self):
        if not self.socket:
            if self.keepconnection:
                try:
                    self.openSocket()
                except Exception, inst:
                    print "checkConnection():",inst
                    self.socket = None
        return not self.socket == None
            
    def parseData(self, data):
        rc = None
        # parse through the data, remember the last (N)ACK
        for d in data:
            t,m = self.parse(d)
            if t == parser.ACK:
                rc = t
            elif t == parser.NACK:
                rc = t
            elif t == parser.UNSOLICITED:
                self.unsolicitedqueue.put(m)
            elif t == parser.MESSAGE:
                self.messagequeue.put(m)
        return rc

    def setReceiveTimeout(self, timeout):
        self.receivetimeout = timeout;

    def receive(self, block):
        starttime = time.time()
        while self.checkConnection():
            r, w, e = select.select(
                [self.socket], [], [], self.receivetimeout)
            
            # is there data to receive?
            if len(r) > 0:
                # will return something
                data = self.socket.recv(1)
                
                if len(data) == 0:
                    # the connection is closed
                    self.eof()
                    return False
                rc = self.parseData(data)
                if not block:
                    return rc
               #if rc is set we got a (N)ACK from the node
                elif rc != None:
                    return rc
            else:
                #the select timed out
                return parser.TIMEOUT

            # a node sending a constant stream of unsolicited
            # messages would never trigger a timeout at the select
            if time.time() - starttime > self.receivetimeout:
                return False

    def sendCommand(self, command, callback=None, errorcallback=None):
        """
        Send a command to the node.

        Before using this operation the node has to be connected
        with connect()
        The command will be sent using the binary uberbus protocol.

        """
        if not self.checkConnection():
            return False

        if self.udp:
            try:
                self.socket.sendto("%s"%(command),(self.address,self.port))
                return True
            except Exception, inst:
                print 'sendCommand():', inst
        else:
            #clear data from old commands
            while not self.messagequeue.empty():
                self.messagequeue.get()
            #send new command
            try:
                self.socket.send("B%c%s"%(len(command),command))
                #block until the command is acked
                if callback == None:
                    rc = self.receive(block = True)
                    return rc == parser.ACK
            except Exception, inst:
                print "sendCommand:", inst
                return False

    def getMessage(self):
        m = None
        while True:
            #discart all but the last message
            while not self.messagequeue.empty():
                m = self.messagequeue.get()
            if m:
                return m
            else:
                #this call block until something is found
                rc = self.receive(block = False)
                if rc == parser.TIMEOUT or rc == False:
                    # When a timeout happens no reply is anticipated
                    return rc

    def process(self):
        while not self.unsolicitedqueue.empty():
            m = self.unsolicitedqueue.get()
            for callback in self.callbacks:
                callback.newUnsolicited(self, m)

    def setTimeout(self, timeout):
        self.timeout = timeout
        if self.dispatcher:
            self.dispatcher.setTimeout(timeout)

    def setTimer(self, time, callback):
        self.time = time
        self.callback = callback
        if self.dispatcher:
            self.disptacher.setTimer(time, callback)

    def checkOnce(self):
        if self.dispatcher == None:
            self.dispatcher = dispatcher.Dispatcher()
            self.dispatcher.addNode(self)
            if self.timeout:
                self.dispatcher.setTimeout(self.timeout)
            if self.time:
                self.dispatcher.setTimer(self.time, self.callback)
        self.dispatcher.checkOnce()

    def checkForever(self):
        while not self.aborted:
            self.checkOnce()

    def abort(self):
        self.aborted = True

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


