#! /usr/bin/python
import sys
import serialinterface
import time
import socket

serial = serialinterface.SerialInterface('/dev/ttyUSB0', 115200, 1)

while True:
    serial.writeMessage("B")
    time.sleep(2)
    m = serial.readMessage();
    if m == 'B':
        break

sock = socket.socket(socket.AF_INET6, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
sock.bind(('', 2323))

while True:
    data = sock.recv(128)

    if len(data) < 30:
        header = '\x01\xff\x08%c'%len(data);
        serial.writeMessage(header + data)
        while serial.readMessage() != 'S':
            pass

