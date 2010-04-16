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

gboolean udp_read(GSocket *socket, GIOCondition condition, gpointer user_data);
gboolean data_udp_read(GSocket *socket, GIOCondition condition, gpointer user_data);
gboolean mgt_udp_read(GSocket *socket, GIOCondition condition, gpointer user_data);

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

gboolean data_udp_read(GSocket *socket, GIOCondition condition, gpointer user_data)
{
    uint8_t buf[100];
    GSocketAddress * src;
    struct node *n = user_data;
    gsize len;
    if( condition == G_IO_IN ){
        len = g_socket_receive_from(socket,&src,(gchar*)buf,100,NULL,NULL);
        if( len ){
            printf("data udp: received:");debug_hexdump(buf,len);printf("\n");
            bus_sendToID(n->id, buf, len, FALSE);
            //TODO: somehow check if the message really gets to the node
            g_socket_send_to(socket,src,"ACK",3,NULL,NULL);
        }else{
            printf("data udp: received empty msg\n");
        }
    }else if( condition == G_IO_ERR ){
        printf("data udp: error\n");
    }else if( condition == G_IO_HUP ){ 
        printf("data udp: hang up\n");
    }else if( condition == G_IO_OUT ){ 
        printf("data udp: out\n");
    }else if( condition == G_IO_PRI ){ 
        printf("date udp: pri\n");
    }else if( condition == G_IO_NVAL ){ 
        printf("data udp: nval\ndropping source\n");
        //drop this source
        return FALSE;
    }else{
        printf("data udp: unkown condition = %d\n",condition);
    }
    return TRUE;
}

gboolean mgt_udp_read(GSocket *socket, GIOCondition condition, gpointer user_data)
{
    uint8_t buf[100];
    GSocketAddress * src;
    struct node *n = user_data;
    gsize len;
    if( condition == G_IO_IN ){
        len = g_socket_receive_from(socket,&src,(gchar*)buf,100,NULL,NULL);
        if( len ){
            printf("mgt udp: received:");debug_hexdump(buf,len);printf("\n");
            busmgt_sendData(n->busadr, buf, len);
            //TODO: somehow check if the message really gets to the node
            g_socket_send_to(socket,src,"ACK",3,NULL,NULL);
        }else{
            printf("mgt udp: received empty msg\n");
        }
    }else if( condition == G_IO_ERR ){
        printf("mgt udp: error\n");
    }else if( condition == G_IO_HUP ){ 
        printf("mgt udp: hang up\n");
    }else if( condition == G_IO_OUT ){ 
        printf("mgt udp: out\n");
    }else if( condition == G_IO_PRI ){ 
        printf("mgt udp: pri\n");
    }else if( condition == G_IO_NVAL ){ 
        printf("mgt udp: nval\ndropping source\n");
        //drop this source
        return FALSE;
    }else{
        printf("mgt udp: unkown condition = %d\n",condition);
    }
    return TRUE;
}


gint net_init(gchar* interface, gchar* baseaddress, gint prefix)
{
    //entries = g_sequence_new(g_free);
    GError * e = NULL;

    net_prefix = prefix;

    g_assert(strlen(interface) < sizeof(net_interface));
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

    GSource *source = g_socket_create_source(udpsocket, G_IO_IN, NULL);
    g_source_set_callback(source, (GSourceFunc)udp_read, NULL, NULL);
    g_source_attach(source, g_main_context_default());

    return 0;
}

void net_createSockets(struct node *n)
{
    GError * err = NULL;
    GInetAddress *addr = n->netadr;

    gchar *tmp = g_inet_address_to_string(addr);
    printf("creating udp sockets on ip: %s\n",tmp);
    g_free(tmp);

    //set up data udp socket
    printf("Creating data udp socket on port 2323\n");
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

    GSource *source = g_socket_create_source(n->udp, G_IO_IN, NULL);
    g_assert(source != NULL);
    g_source_set_callback(source, (GSourceFunc)data_udp_read, n, NULL);
    g_source_attach(source, g_main_context_default ());

    //set up mgt udp socket
    printf("Creating mgt udp socket on port 2324\n");
    sa = g_inet_socket_address_new(addr,2324);
    n->mgtudp = g_socket_new(G_SOCKET_FAMILY_IPV6,
                        G_SOCKET_TYPE_DATAGRAM,
                        G_SOCKET_PROTOCOL_UDP,
                        NULL);

    g_assert(n->mgtudp != NULL);

    if( g_socket_bind(n->mgtudp,sa,TRUE,&err) == FALSE ){
        fprintf(stderr, "error while binding socket: %s\n", err->message);
        g_error_free(err);
    }

    source = g_socket_create_source(n->mgtudp, G_IO_IN, NULL);
    g_assert(source != NULL);
    g_source_set_callback(source, (GSourceFunc)mgt_udp_read, n, NULL);
    g_source_attach(source, g_main_context_default ());

    n->netup = TRUE;
    //FIXME: unref address results in segfault?
    //g_object_unref(sa);
    
}

void net_removeSockets(struct node *n)
{
    GError * err = NULL;
    printf("removing sockets of id %s\n",n->id);
    //remove data udp socket
    //gboolean rc = g_socket_shutdown(n->udp, FALSE, FALSE, &err);
    gboolean rc = g_socket_close(n->udp, &err);
    if( rc  == TRUE ){
        printf("success\n");
    }else{
        fprintf(stderr, "error in g_socket_shutdown: %s\n", err->message);
        g_error_free(err);
    }
    g_object_unref(n->udp);
    //FIXME: unref GSource also
    
    //remove mgt udp socket
    rc = g_socket_close(n->mgtudp, &err);
    if( rc  == TRUE ){
        printf("success\n");
    }else{
        fprintf(stderr, "error in g_socket_shutdown: %s\n", err->message);
        g_error_free(err);
    }
    g_object_unref(n->mgtudp);

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

