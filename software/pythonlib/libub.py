#import socket
#bus = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)

class UBNode:
    def __init__(self, name):
        self.name = name
    def connect(self):
        return True

    def sendCommand(self, command):
        pass
