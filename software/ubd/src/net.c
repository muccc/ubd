#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include <gio/gio.h>

#include "debug.h"
#include "address6.h"
#include "interface.h"
#include "bus.h"
#include "cmdparser.h"
#include "nodes.h"
#include "net_tcp.h"
#include "net_udp.h"

GInetAddress    *net_base;
GSocket         *udpsocket;
gint            net_prefix;

gint net_init(gchar* interface, gchar* baseaddress, gint prefix)
{
    GError * e = NULL;
    interface = NULL;   //prevent warning

    net_prefix = prefix;
    net_base = g_inet_address_new_from_string(baseaddress);
    if( net_base == NULL ){
        fprintf(stderr, "net_init: Could not parse base address");
        return -1;
    }
    
    address6_init(net_base);
    tcp_init();

    printf("net_init: Creating udp socket on port 2323\n");
    GSocketAddress * sa = g_inet_socket_address_new(net_base,2323);
    udpsocket = g_socket_new(G_SOCKET_FAMILY_IPV6,
                        G_SOCKET_TYPE_DATAGRAM,
                        G_SOCKET_PROTOCOL_UDP,
                        &e);
    if( udpsocket == NULL && e != NULL ){
        fprintf(stderr, "net_init: Error while creating udp socket: %s\n", e->message);
        g_error_free(e);
        e = NULL;
    }
    if( udpsocket == NULL ){
        fprintf(stderr, "net_init: g_socket_new failed\n");
        return -1;
    }
    if( g_socket_bind(udpsocket,sa,TRUE,&e) == FALSE ){
        fprintf(stderr, "net_init: Error while binding udp socket: %s\n", e->message);
        g_error_free(e);
        e = NULL;
    }
    GSource *source = g_socket_create_source(udpsocket, G_IO_IN, NULL);
    g_source_set_callback(source, (GSourceFunc)udp_read, NULL, NULL);
    g_source_attach(source, g_main_context_default());

    //set up data tcp listener
    printf("net_init: Creating tcp socket on port 2323\n");
    GSocketService *gss = g_socket_service_new();
    //TODO: save a reference to the gss somewhere
    if( g_socket_listener_add_address(G_SOCKET_LISTENER(gss), sa,
        G_SOCKET_TYPE_STREAM, G_SOCKET_PROTOCOL_TCP, NULL, NULL, &e)
            == FALSE ){
        fprintf(stderr, "net_init: error while creating socket listener: %s\n",
                e->message);
        g_error_free(e);
    }
    g_signal_connect(gss, "incoming", G_CALLBACK(tcp_listener),NULL);
    g_socket_service_start(gss);
 
    g_object_unref(sa);


    return 0;
}

void net_createSockets(struct node *n)
{
    GError * err = NULL;
    GInetAddress *addr = n->netadr;
    GSource *source;

    gchar *tmp = g_inet_address_to_string(addr);
    printf("net_createSockets: Creating sockets on ip: %s\n",tmp);
    g_free(tmp);

    //set up data udp socket
    printf("net_createSockets: Creating udp socket on port 2323\n");
    GSocketAddress * sa = g_inet_socket_address_new(addr,2323);
    n->udp = g_socket_new(G_SOCKET_FAMILY_IPV6,
                        G_SOCKET_TYPE_DATAGRAM,
                        G_SOCKET_PROTOCOL_UDP,
                        NULL);

    g_assert(n->udp != NULL);

    if( g_socket_bind(n->udp, sa, TRUE, &err) == FALSE ){
        fprintf(stderr, "net_createSockets: Error while binding udp socket: %s\n", err->message);
        g_error_free(err);
        return;
    }

    source = g_socket_create_source(n->udp, G_IO_IN, NULL);
    g_assert(source != NULL);
    g_source_set_callback(source, (GSourceFunc)udp_read, n, NULL);
    g_source_attach(source, g_main_context_default());

    printf("net_createSockets: Creating tcp socket on port 2323\n");
    GSocketService *gss = g_socket_service_new();
    if( g_socket_listener_add_address(G_SOCKET_LISTENER(gss), sa,
        G_SOCKET_TYPE_STREAM, G_SOCKET_PROTOCOL_TCP, NULL, NULL, &err)
            == FALSE ){
        fprintf(stderr, "net_createSockets: Error while creating socket listener: %s\n", err->message);
        g_error_free(err);
        return;
    }
    g_signal_connect(gss, "incoming", G_CALLBACK(tcp_listener), n);
    g_socket_service_start(gss);
 
    printf("net_createSockets: activating network for this node\n");
    n->netup = TRUE;
    //FIXME: unref address results in segfault?
    //g_object_unref(sa); 
}

void net_removeSockets(struct node *n)
{
    GError *err = NULL;
    printf("net_removeSockets: Closing sockets of node %s\n",n->id);
    //remove data udp socket
    //gboolean rc = g_socket_shutdown(n->udp, FALSE, FALSE, &err);
    printf("net_removeSockets: Closing udp socket\n");
    gboolean rc = g_socket_close(n->udp, &err);
    if( rc  == TRUE ){
        printf("success\n");
    }else{
        fprintf(stderr, "error in g_socket_close: %s\n", err->message);
        g_error_free(err);
    }
    g_object_unref(n->udp);
    //FIXME: unref GSource also

}

