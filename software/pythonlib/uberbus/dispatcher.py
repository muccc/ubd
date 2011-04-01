import time
import select
import socket

class Dispatcher:
    def __init__(self):
        self.nodes = []
        self.timeout = 10
        self.time = 0
        self.callback = None
        self.aborted = False

    def addNode(self, node):
        self.nodes.append(node)
    
    def removeNode(self, node):
        for i in self.nodes.count(node):
            self.nodes.remove(node)

    def setTimeout(self, timeout):
        self.timeout = timeout

    def checkOnce(self, timeout = None):
        if not timeout:
            timeout = self.timeout
        stoptime = time.time() + timeout
        # loop as long as select returns valid data
        while True:
            starttime = time.time()
            sockets = dict([(node.getSocket(), node)
                for node in self.nodes if node.checkConnection()])
            print 'select', sockets.keys()
            r, w, e = select.select(
                sockets.keys(), [], [], stoptime - starttime)
            print r, w, e
            for socket in r:
                node = sockets[socket]
                data = socket.recv(1)
                #print 'recv', list(data)
                if len(data) == 0:
                    node.eof()
                else:
                    node.parseData(data)
            
            for node in self.nodes:
                node.process()

            # a node sending a constant stream of unsolicited
            # messages would never trigger a timeout at the select
            if timeout == 0 or time.time() >= stoptime:
                return

            if len(r) == 0:
                return

    def setTimer(self, time, callback):
        self.time = time
        self.callback = callback

    def checkForever(self):
        starttime = time.time()
        while not self.aborted:
            if self.time != 0:
                timeout = starttime + self.time - time.time()
                if timeout > self.timeout:
                    self.checkOnce()
                else:
                    self.checkOnce(timeout)
                if time.time() - starttime >= self.time:
                    self.callback()
                    starttime = time.time()
            else:
                self.checkOnce()

        self.aborted = False

    def abort(self):
        self.aborted = True

