#! /usr/bin/python
import sys
import serialinterface
import time
import socket
import thread
import threading

serial = serialinterface.SerialInterface('/dev/ttyUSB0', 115200, 1)
ack = threading.Lock()

def read():
    global ack
    while 1:
        m = serial.readMessage()
        if m == 'S':
            #print "packet got acked"
            ack.release()

while True:
    serial.writeMessage("B")
    time.sleep(2)
    m = serial.readMessage();
    if m == 'B':
        break
thread.start_new_thread(read,())
sock = socket.socket(socket.AF_INET6, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
sock.bind(('', 2323))

while True:
    data = sock.recv(128)

    if len(data) < 30:
        header = '\x01\xff\x08%c'%len(data);
        ack.acquire()
        serial.writeMessage(header + data)
