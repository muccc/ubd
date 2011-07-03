import ubnode

class DigitalOutput(ubnode.UBNode):
    def __init__(self, address):
        ubnode.UBNode.__init__(self,address,2311)

    def set(self, pin):
        cmd = "s %s 1"%pin
        return self.sendCommand(cmd)

    def clear(self, pin):
        cmd = "s %s 0"%pin
        return self.sendCommand(cmd)
