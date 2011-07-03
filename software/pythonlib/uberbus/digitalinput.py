import ubnode

class DigitalInput(ubnode.UBNode):
    def __init__(self, address):
        ubnode.UBNode.__init__(self,address,2312)

class DigitalInputCallback:
    def newUnsolicited(self, node, data):
        print "digitalinput callback:", list(data)
        self.onUnsolicited(node, data)
        args = data.split()
        if args[0] == 'I':
            match = True
            self.onInput(node, args[1], int(args[2]))
        else:
            self.onUnmatchedUnsolicited(node, data)

    def onInput(self, node, pin, state):
        pass
    
    def onUnmatchedUnsolicited(self, node, data):
        pass

    def onUnsolicited(self, node, data):
        pass

