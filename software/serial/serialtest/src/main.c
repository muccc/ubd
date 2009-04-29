#include <config.h>
#include <stdio.h>
#include <gnet.h>


int main (int argc, char *argv[])
{
    gnet_init();
    gnet_ipv6_set_policy(GIPV6_POLICY_IPV6_ONLY);
    
    GInetAddr* iface = gnet_inetaddr_new(argv[2],2323);
    if( iface == NULL ){
        printf("not a valid ipv6 address\n");
        return;
    }
    GUdpSocket*  s = gnet_udp_socket_new_full(iface,0);
    g_assert(s!=NULL);
    if( s == NULL ){
        printf("could not create socket\n");
        return;
    }
//  usleep(3*1000*1000);
    GInetAddr* a = gnet_inetaddr_new(argv[1],2323);
    gnet_udp_socket_send(s,"blubb",5,a);
    puts ("Hello World!");
    puts ("This is " PACKAGE_STRING ".");
//    usleep(10*1000*1000);
    return 0;
}

