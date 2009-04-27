import socket
from logger import flogger,getLogger

class NetInterface:
    conn = ''
    addr = ''
    log = getLogger('SerialInterface')
    def __init__(self, port=2323, host=''):
        self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.s.bind(('', 2323))
        self.s.listen(1)
    def readMessage(self):
        if not self.conn:
            self.conn, self.addr = self.s.accept()
            print 'Connected by', self.addr
        
        data = ""
        escaped = False
        stop = False
        start = False

        while True:
            c = self.conn.recv(1)
            if len(c) == 0:             #A timout occured
                self.log.warning('TIMEOUT')
                self.conn, self.addr = self.s.accept()
                print 'Connected by', self.addr
                continue
            #print "c=", list(c)
        #    continue
            if escaped:
                if c == '0':
                    start = True
                elif c == '1':
                    stop = True
                elif c == '\\':
                    d = '\\'
                escaped = False
            elif c == '\\':
                escaped = 1
            else:
                d = c
                
            if start:
                start = False
            elif stop:
                if data[0] == 'D':
                    self.log.info('Debug: %s'%(data[1:]))
                    data = ""
                    stop = False
                else:
                    self.log.debug('received message: len=%d data=%s'%(len(data), list(data)))
                    return data
            elif escaped == False:
                data += str(d)

    def writeMessage(self, msg):
        print "wriring:"+msg
        self.conn.send(msg)

