#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include <gio/gio.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "address6db.h"

GSocket *multicast_createSocket(gchar *groupname, guint port,
                                 GSocketAddress **sa)
{
    GError *err = NULL;
    GInetAddress *addr = address6db_getMulticastAddr(groupname);
    *sa = g_inet_socket_address_new(addr,port);

    GSocket *socket = g_socket_new(G_SOCKET_FAMILY_IPV6,
                        G_SOCKET_TYPE_DATAGRAM,
                        G_SOCKET_PROTOCOL_UDP,
                        NULL);
    g_assert(socket != NULL);

    if( g_socket_bind(socket, *sa, TRUE, &err) == FALSE ){
        fprintf(stderr, "net_createSockets: Error while binding udp socket: %s\n", err->message);
        g_error_free(err);
        g_object_unref(*sa);
        g_object_unref(socket);
        return NULL;
    }

    struct addrinfo *resmulti;
    struct ipv6_mreq mreq;
    mreq.ipv6mr_interface = if_nametoindex("eth1");
    getaddrinfo("ff18::1", NULL, NULL, &resmulti);
    mreq.ipv6mr_multiaddr = ((struct sockaddr_in6 *)resmulti->ai_addr)->sin6_addr;
    setsockopt(g_socket_get_fd(socket), IPPROTO_IPV6,
        IPV6_ADD_MEMBERSHIP, (char *)&mreq, sizeof(mreq));
    return socket;
}
