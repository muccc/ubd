class Packet:
    def __init__(self):
        pass

    def __init__(self,msg):
        if len(msg) > 1 and msg[0] == 'P':
            self.src = ord(msg[1])
            self.dest = ord(msg[2])
            self.flags = ord(msg[3])
            self.seq = ord(msg[4])
            self.len = ord(msg[5])
            self.data = msg[6:]
            self.valid = True
        else:
            self.valid = False

    def __str__(self):
        return "Packet: src=%d dest=%d flags=0x%x seq=%d len=%d data=%s"%(self.src,self.dest,self.flags,self.seq,self.len,str(list(self.data)))
    def getMessage(self):
        return 'P'+chr(self.src)+chr(self.dest)+chr(self.flags)+chr(self.seq)+chr(self.len)+self.data

if __name__ == "__main__":
    msg = "012345678"
    print "msg="+msg
    p = Packet(msg)
    print p
