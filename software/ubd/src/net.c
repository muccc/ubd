#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include <gio/gio.h>
#include <syslog.h>

#include "debug.h"
#include "address6.h"
#include "interface.h"
#include "bus.h"
#include "cmdparser.h"
#include "nodes.h"
#include "net_tcp.h"
#include "net_udp.h"
#include "broadcast.h"
#include "nodes.h"

GInetAddress    *net_base;
GInetAddress    *net_multicastbase;
GSocket         *udpsocket;

gint net_init(gchar* interface, gchar* baseaddress, gchar *multicastbase)
{
    GError * e = NULL;
    interface = NULL;   //prevent warning

    net_base = g_inet_address_new_from_string(baseaddress);
    if( net_base == NULL ){
        syslog(LOG_ERR, "net_init: Could not parse base address");
        return -1;
    }
    struct node n;
    n.netadr = net_base;
    n.ubnetd = NULL;
    syslog(LOG_DEBUG,"creating base address...\n");
    interface_createAddress(&n);
    usleep(3000*1000);
    syslog(LOG_DEBUG,"done\n");
    net_multicastbase = g_inet_address_new_from_string(multicastbase);
    if( net_multicastbase == NULL ){
        syslog(LOG_ERR, "net_init: Could not parse multicast base address");
        return -1;
    }   
    address6_init(net_base, net_multicastbase);
    tcp_init();

    GSocketAddress * sa = g_inet_socket_address_new(net_base,2323);
    //set up data tcp listener
    syslog(LOG_DEBUG,"net_init: Creating tcp socket on port 2323\n");
    GSocketService *gss = g_socket_service_new();
    //TODO: save a reference to the gss somewhere
    if( g_socket_listener_add_address(G_SOCKET_LISTENER(gss), sa,
        G_SOCKET_TYPE_STREAM, G_SOCKET_PROTOCOL_TCP, NULL, NULL, &e)
            == FALSE ){
        syslog(LOG_WARNING, "net_init: error while creating socket listener: %s\n",
                e->message);
        g_error_free(e);
    }
    g_signal_connect(gss, "incoming", G_CALLBACK(tcp_listener),NULL);
    g_socket_service_start(gss);
 
    g_object_unref(sa);

    return 0;
}

static gboolean net_createUDPSocket(struct node *n, guint classid)
{
    GError * err = NULL;
    GInetAddress *addr = n->netadr;
    guint port = 2300+n->classes[classid];

    syslog(LOG_DEBUG,"net_createSockets: Creating udp socket on port %u\n", port);

    GSocketAddress * sa = g_inet_socket_address_new(addr,port);

    n->udpsockets[classid].n = n;
    GSocket *socket = g_socket_new(G_SOCKET_FAMILY_IPV6,
                        G_SOCKET_TYPE_DATAGRAM,
                        G_SOCKET_PROTOCOL_UDP,
                        NULL);

    g_assert(socket != NULL);

    if( g_socket_bind(socket, sa, TRUE, &err) == FALSE ){
        syslog(LOG_WARNING, "net_createSockets: Error while binding udp socket: %s\n", err->message);
        g_error_free(err);
        return FALSE;
    }

    GSource *source = g_socket_create_source(socket, G_IO_IN, NULL);
    g_assert(source != NULL);
    g_source_set_callback(source, (GSourceFunc)udp_read,
            &(n->udpsockets[classid]) , NULL);
    g_source_attach(source, g_main_context_default());
    n->udpsockets[classid].socket = socket;
    n->udpsockets[classid].source = source;
    n->udpsockets[classid].socketaddress = sa;
    n->udpsockets[classid].classid = classid;

    //broadcast_addService(n->classes[classid]);
    return TRUE;
}


static void net_removeUDPSocket(struct node *n, guint classid)
{
    GError *err = NULL;
    syslog(LOG_DEBUG,"net_removeSockets: Closing udp socket\n");
    g_source_destroy(n->udpsockets[classid].source);
    g_source_unref(n->udpsockets[classid].source);

    //gboolean rc = g_socket_shutdown(n->udp, FALSE, FALSE, &err);
    gboolean rc = g_socket_close(n->udpsockets[classid].socket, &err);
    if( rc  == TRUE ){
        syslog(LOG_DEBUG,"success\n");
    }else{
        //TODO: log error
        syslog(LOG_WARNING, "error in g_socket_close: %s\n", err->message);
        g_error_free(err);
    }
    g_object_unref(n->udpsockets[classid].socket);
}

static gboolean net_createTCPSocket(struct node *n, guint classid)
{
    guint port = 2300+n->classes[classid];
    GError * err = NULL;
    GInetAddress *addr = n->netadr;
    syslog(LOG_DEBUG,"net_createSockets: Creating tcp socket listener "
           "on port %u\n", port);

    GSocketAddress *sa = g_inet_socket_address_new(addr,port);
    GSocketService *gss = g_socket_service_new();

    if( g_socket_listener_add_address(G_SOCKET_LISTENER(gss), sa,
        G_SOCKET_TYPE_STREAM, G_SOCKET_PROTOCOL_TCP, NULL, NULL, &err)
            == FALSE ){
        syslog(LOG_WARNING, "net_createSockets: Error while creating"
                        "socket listener: %s\n", err->message);
        g_error_free(err);
        g_object_unref(gss);
        return FALSE;
    }
    n->tcpsockets[classid].n = n;
    n->tcpsockets[classid].socketservice = gss;
    n->tcpsockets[classid].socketaddress = sa;
    n->tcpsockets[classid].classid = classid;
    g_signal_connect(gss, "incoming", G_CALLBACK(tcp_listener),
                     &(n->tcpsockets[classid]));
    g_socket_service_start(gss);
    return TRUE;
}

static void net_removeTCPSocketListener(struct node *n, guint classid)
{
    g_socket_service_stop(n->tcpsockets[classid].socketservice);
    g_socket_listener_close(
            G_SOCKET_LISTENER(n->tcpsockets[classid].socketservice));
    g_object_unref(n->tcpsockets[classid].socketservice);
    //FIXME: unref address results in segfault?
    //g_object_unref(sa); 

}

void net_createSockets(struct node *n)
{
    GInetAddress *addr = n->netadr;
    gchar *tmp = g_inet_address_to_string(addr);
    syslog(LOG_DEBUG,"net_createSockets: Creating sockets on ip: %s\n",tmp);
    g_free(tmp);

    guint i;
    for(i=0; i<sizeof(n->classes); i++){
        if( n->classes[i] != 0 ){
            if( !(net_createUDPSocket(n, i) &&
                  net_createTCPSocket(n, i)) )
                    return;
        }
    }

    syslog(LOG_DEBUG,"net_createSockets: Creating tcp management socket listener on port 2324\n");
    GError * err = NULL;
    GSocketAddress * samgt = g_inet_socket_address_new(addr,2324);
    GSocketService *gss = g_socket_service_new();

    if( g_socket_listener_add_address(G_SOCKET_LISTENER(gss), samgt,
        G_SOCKET_TYPE_STREAM, G_SOCKET_PROTOCOL_TCP, NULL, NULL, &err)
            == FALSE ){
        syslog(LOG_WARNING, "net_createSockets: Error while creating socket listener: %s\n", err->message);
        g_error_free(err);
        g_object_unref(gss);
        return;
    }
    n->mgtsocket.n = n;
    n->mgtsocket.socketservice = gss;
    n->mgtsocket.socketaddress = samgt;
    g_signal_connect(gss, "incoming", G_CALLBACK(tcp_listener),
                                        &(n->mgtsocket));
    g_socket_service_start(gss);

    syslog(LOG_DEBUG,"net_createSockets: activating network for this node\n");
   
    avahi_registerServices(n);
    n->netup = TRUE;
}

void net_removeSockets(struct node *n)
{
    avahi_removeServices(n);
    syslog(LOG_DEBUG,"net_removeSockets: Closing sockets of node %s\n",n->id);

    guint i;
    for(i=0; i<sizeof(n->classes); i++){
        if( n->classes[i] != 0 ){
            net_removeUDPSocket(n, i);
            net_removeTCPSocketListener(n, i);
        }
    }
    
    g_socket_service_stop(n->mgtsocket.socketservice);
    g_socket_listener_close(G_SOCKET_LISTENER(n->mgtsocket.socketservice));
    g_object_unref(n->mgtsocket.socketservice);
}
