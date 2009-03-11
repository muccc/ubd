#include <config.h>
#include <stdio.h>
#include <gnet.h>


int main (int argc, char *argv[])
{
    gnet_init();
    GUdpSocket*  s = gnet_udp_socket_new();
//    usleep(3*1000*1000);
    GInetAddr* a = gnet_inetaddr_new(argv[1],2323);
    gnet_udp_socket_send(s,"blubb",5,a);
    puts ("Hello World!");
    puts ("This is " PACKAGE_STRING ".");
//    usleep(10*1000*1000);
    return 0;
}

