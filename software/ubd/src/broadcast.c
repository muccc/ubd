#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include <gio/gio.h>

#include "debug.h"
#include "address6.h"
#include "interface.h"
#include "config.h"
#include "nodes.h"
#include "broadcast.h"
#include "bus.h"

GHashTable * table;

void broadcast_init(void)
{
    table = g_hash_table_new(NULL,NULL);
}

gboolean broadcast_addService(gint service)
{
    GError *err = NULL;
    GInetAddress *addr = g_inet_address_new_from_string(config.base);
    guint port = 2300+service;
    
    //check if we already have a broadcast socket for this service
    if( g_hash_table_lookup(table, GINT_TO_POINTER(service)) != NULL )
        return TRUE;

    printf("net_createSockets: Creating broadcast udp socket on port %u\n", port);
    GSocketAddress * sa = g_inet_socket_address_new(addr,port);

    GSocket *socket = g_socket_new(G_SOCKET_FAMILY_IPV6,
                        G_SOCKET_TYPE_DATAGRAM,
                        G_SOCKET_PROTOCOL_UDP,
                        NULL);

    g_assert(socket != NULL);

    if( g_socket_bind(socket, sa, TRUE, &err) == FALSE ){
        fprintf(stderr, "net_createSockets: Error while binding udp socket: %s\n", err->message);
        g_error_free(err);
        return FALSE;
    }

    GSource *source = g_socket_create_source(socket, G_IO_IN, NULL);
    g_assert(source != NULL);
    g_source_set_callback(source, (GSourceFunc)broadcast_read,
            GINT_TO_POINTER(service) , NULL);
    g_source_attach(source, g_main_context_default());
    g_hash_table_insert(table, GINT_TO_POINTER(service), source);
    return TRUE;
}

gboolean broadcast_read(GSocket *socket, GIOCondition condition,
                                        gpointer user_data)
{
    uint8_t buf[100];
    GSocketAddress * src;
    gint class = GPOINTER_TO_INT(user_data);
    gssize len;    
    if( condition == G_IO_IN ){
        len = g_socket_receive_from(socket,&src,(gchar*)buf,
                                sizeof(buf),NULL,NULL);
        if( len > 0 ){
            printf("udp_read: Received:");
            debug_hexdump(buf,len);
            printf("\n");
            bus_sendToClass(class, buf, len);
        }else{
            printf("udp_read: Error while receiving: len=%d\n",len);
        }
    }else{
        printf("udp_read: Received ");
        if( condition == G_IO_ERR ){
            printf("G_IO_ERR\n");
        }else if( condition == G_IO_HUP ){ 
            printf("G_IO_HUP\n");
        }else if( condition == G_IO_OUT ){ 
            printf("G_IO_OUT\n");
        }else if( condition == G_IO_PRI ){ 
            printf("G_IO_PRI\n");
        }else if( condition == G_IO_NVAL ){ 
            printf("G_IO_NVAL\ndropping source\n");
            return FALSE;
        }else{
            printf("unkown condition = %d\n",condition);
        }
    }
    return TRUE;
}
