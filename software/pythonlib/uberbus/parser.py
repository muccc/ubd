NONE = 1
ACK = 2
NACK = 3
MESSAGE = 4
UNSOLICITED = 5
TIMEOUT = 6

class Parser:
    """
    A parser for uberbus message streams

    """

    def __init__(self):
        self.state = 0

    def parse(self, data):
        """
        Parse a data stream.

        Parse the character data.

        Return a tuple (messagetype, messagedata)
        Where message type can be:
        uberbus.parser.NONE
        uberbus.parser.ACK
        uberbus.parser.NACK
        uberbus.parser.MESSAGE
        uberbus.parser.UNSOLICITED

        """
        if self.state == 0:
            if data == 'C':
                self.state = 1
                self.msg = ''
                self.messagetype = MESSAGE
            elif data == 'B':
                self.state = 2
                self.messagetype = MESSAGE
            elif data == 'c':
                self.state = 1
                self.msg = ''
                self.messagetype = UNSOLICITED
            elif data == 'b':
                self.state = 2
                self.msg = ''
                self.messagetype = UNSOLICITED
            elif data == 'A':
                return  (ACK, '')
            elif data == 'N':
                return (NACK,'')
        elif self.state == 1:
            if data == '\n' or data == '\r':
                self.state = 0
                return (self.messagetype, self.msg)
            else:
                self.msg += data
        elif self.state == 2:
            self.msglen = ord(data)
            self.state = 3
        elif self.state == 3:
            self.msg += data
            if self.msglen == len(self.msg):
                self.state = 0
                return (self.messagetype, self.msg)
        return (NONE, '')
