import struct
import socket
import sys


addrinfo = socket.getaddrinfo("ff18:583:786d:8ec9:d3d6:fd2b:1155:e066", None)[0]

s = socket.socket(addrinfo[0], socket.SOCK_DGRAM)

ttl_bin = struct.pack('@i', 1)
s.setsockopt(socket.IPPROTO_IPV6, socket.IPV6_MULTICAST_HOPS, ttl_bin)

n = 0
while True:
    line = '{"cmd":"update-service", "service-type":"foobar", "id":"%d", "url":"somewhere","name":"einname", "port":42}'%n
    n+=1
    s.sendto(line, (addrinfo[4][0], 2323))

