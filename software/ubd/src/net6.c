#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include <gio/gio.h>

#include "debug.h"
#include "busmgt.h"
#include "mgt.h"
#include "address6.h"
#include "interface.h"
#include "bus.h"

//GSequence * entries;

GInetAddress*  net_base;
GSocket*  udpsocket;
gint        net_prefix;
char        net_interface[1024];

gboolean udp_read(GSocket *socket, GIOCondition condition, gpointer user_data)
{
    //g_file_new_for_path("blubb");
    uint8_t buf[100];
    gsize len;
    GSocketAddress * src;
    condition = 0;
    user_data = NULL;
    len = g_socket_receive_from(socket,&src,(gchar*)buf,100,NULL,NULL);
    if( len ){
        printf("udp: received:");debug_hexdump(buf,len);printf("\n");
        g_socket_send_to(socket,src,"ACK",3,NULL,NULL);
    }else{
        printf("udp: received empty msg\n");
    }
    return TRUE;
}

gboolean entry_udp_read(GSocket *socket, GIOCondition condition, gpointer user_data)
{
    uint8_t buf[100];
    GSocketAddress * src;
    struct node *n = user_data;
    condition = 0;

    gsize len;
    len = g_socket_receive_from(socket,&src,(gchar*)buf,100,NULL,NULL);
    if( len ){
        printf("udp: received:");debug_hexdump(buf,len);printf("\n");
        bus_sendToID(n->id, buf, len, FALSE);
        g_socket_send_to(socket,src,"ACK",3,NULL,NULL);
    }else{
        printf("udp: received empty msg\n");
    }
    return TRUE;
}

gint net_init(gchar* interface, gchar* baseaddress, gint prefix)
{
    //entries = g_sequence_new(g_free);
    GError * e = NULL;

    net_prefix = prefix;

    strcpy(net_interface, interface);
    net_base = g_inet_address_new_from_string(baseaddress);
    if( net_base == NULL ){
        printf("could not parse base address");
        return -1;
    }
    
    address6_init(net_base);
    interface_init(interface);

    GSocketAddress * sa = g_inet_socket_address_new(net_base,2323);
    udpsocket = g_socket_new(G_SOCKET_FAMILY_IPV6,
                        G_SOCKET_TYPE_DATAGRAM,
                        G_SOCKET_PROTOCOL_UDP,
                        &e);
    if( udpsocket == NULL && e != NULL ){
        fprintf(stderr, "error while creating socket: %s\n", e->message);
        g_error_free(e);
    }
    
    if( udpsocket == NULL ){
        printf("g_socket_new failed\n");
        return -1;
    }
    if( g_socket_bind(udpsocket,sa,TRUE,&e) == FALSE ){
        fprintf(stderr, "error while binding socket: %s\n", e->message);
        g_error_free(e);
    }
    g_object_unref(sa);
    GSource *s = g_socket_create_source(udpsocket, G_IO_IN, NULL);
    g_source_set_callback(s, (GSourceFunc)udp_read, NULL, NULL);
    g_source_attach(s, g_main_context_default ());

    return 0;
}

void net_createSockets(struct node *n)
{
    GError * err = NULL;
    GInetAddress *addr = n->netadr;
    gchar *tmp = g_inet_address_to_string(addr);

    printf("creating udp socket on ip: %s\n",tmp);
    g_free(tmp);

    GSocketAddress * sa = g_inet_socket_address_new(addr,2323);
    n->udp = g_socket_new(G_SOCKET_FAMILY_IPV6,
                        G_SOCKET_TYPE_DATAGRAM,
                        G_SOCKET_PROTOCOL_UDP,
                        NULL);

    g_assert(n->udp != NULL);

    if( g_socket_bind(n->udp,sa,TRUE,&err) == FALSE ){
        fprintf(stderr, "error while binding socket: %s\n", err->message);
        g_error_free(err);
    }

    GSource *s = g_socket_create_source(n->udp, G_IO_IN, NULL);
    g_assert(s != NULL);
    g_source_set_callback(s, (GSourceFunc)entry_udp_read, n, NULL);
    g_source_attach(s, g_main_context_default ());

    n->netup = TRUE;
    //FIXME: free address
    //g_object_unref(sa);
}

void net_removeSockets(struct node *n)
{
    g_assert( g_socket_shutdown(n->udp, TRUE, TRUE, NULL) == TRUE );
}

/*struct entry * net_getEntryById(gchar * id)
{
    GSequenceIter * i;
    for( i=g_sequence_get_begin_iter(entries);
            i!=g_sequence_get_end_iter(entries); 
            i=g_sequence_iter_next(i) ){
        struct entry *e = g_sequence_get(i);
        if( e->id != NULL && strcmp(id, e->id) == 0 ){
            return e;
        }
    }
    return NULL;
}*/

