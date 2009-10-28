#include <gnet.h>
#include <stdint.h>
#include <stdio.h>

#include "debug.h"

GIOChannel * udpio;
GInetAddr* iface;
GUdpSocket*  udpsocket;

gboolean udp_read(GIOChannel* serial, GIOCondition condition, gpointer data)
{
    uint8_t buf[100];
    gsize len;
    GInetAddr * src;
    serial = NULL;
    condition = 0;
    data = NULL;
    len = gnet_udp_socket_receive(udpsocket, (char*)buf, 100, &src);
    if( len ){
        printf("udp: received:");debug_hexdump(buf,len);printf("\n");
    }else{
        printf("udp: received empty msg\n");
    }
    return TRUE;
}

gint net_init(gchar* interface)
{
    gnet_init();
    gnet_ipv6_set_policy(GIPV6_POLICY_IPV6_ONLY);

    iface = gnet_inetaddr_new(interface, 0);
    if( iface == NULL ){
        return -1;
    }

    udpsocket = gnet_udp_socket_new_full(iface, 2323);
    if( udpsocket == NULL ){
        return -1;
    }

    udpio = gnet_udp_socket_get_io_channel(udpsocket);
    g_io_add_watch(udpio,G_IO_IN,udp_read,NULL);
    return 0;
}


