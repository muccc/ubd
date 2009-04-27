class Packet:
    def __init__(self,msg):
        self.src = ord(msg[0])
        self.dest = ord(msg[1])
        self.flags = ord(msg[2])
        self.seq = ord(msg[3])
        self.len = ord(msg[4])
        self.data = msg[5:]

    def __str__(self):
        return "Packet: src=%d dest=%d flags=0x%x seq=%d len=%d data=%s"%(self.src,self.dest,self.flags,self.seq,self.len,str(list(self.data)))

if __name__ == "__main__":
    msg = "012345678"
    print "msg="+msg
    p = Packet(msg)
    print p
