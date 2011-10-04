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

    syslog(LOG_DEBUG,"net_createSockets: Creating broadcast udp socket on port %u\n", port);
    GSocketAddress * sa = g_inet_socket_address_new(addr,port);

    GSocket *socket = g_socket_new(G_SOCKET_FAMILY_IPV6,
                        G_SOCKET_TYPE_DATAGRAM,
                        G_SOCKET_PROTOCOL_UDP,
                        NULL);

    ub_assert(socket != NULL);

    if( g_socket_bind(socket, sa, TRUE, &err) == FALSE ){
        syslog(LOG_WARNING, "net_createSockets: Error while binding udp socket: %s\n", err->message);
        g_error_free(err);
        return FALSE;
    }

    GSource *source = g_socket_create_source(socket, G_IO_IN, NULL);
    ub_assert(source != NULL);
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
            syslog(LOG_DEBUG,"udp_read: Received:");
            debug_hexdump(buf,len);
            syslog(LOG_DEBUG,"\n");
            bus_sendToClass(class, buf, len);
        }else{
            syslog(LOG_WARNING,"udp_read: Error while receiving: len=%d\n",len);
        }
    }else{
        syslog(LOG_DEBUG,"udp_read: Received ");
        if( condition == G_IO_ERR ){
            syslog(LOG_DEBUG,"G_IO_ERR\n");
        }else if( condition == G_IO_HUP ){ 
            syslog(LOG_DEBUG,"G_IO_HUP\n");
        }else if( condition == G_IO_OUT ){ 
            syslog(LOG_DEBUG,"G_IO_OUT\n");
        }else if( condition == G_IO_PRI ){ 
            syslog(LOG_DEBUG,"G_IO_PRI\n");
        }else if( condition == G_IO_NVAL ){ 
            syslog(LOG_DEBUG,"G_IO_NVAL\ndropping source\n");
            return FALSE;
        }else{
            syslog(LOG_WARNING,"unkown condition = %d\n",condition);
        }
    }
    return TRUE;
}
