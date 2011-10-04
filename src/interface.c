#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <glib.h>
#include <gio/gio.h>
#include <syslog.h>

#include "interface.h"

void interface_init()
{
}

gint interface_createAddress(struct node *n) //GInetAddress *addr)
{
    GError * e = NULL;
    
    GInetAddress *lo = g_inet_address_new_loopback(
                            G_SOCKET_FAMILY_IPV6);
    GSocketAddress *sa = g_inet_socket_address_new(
                            lo,
                            42);
    
    GSocket *s = n->ubnetd;
    if( s != NULL ){
        g_socket_close(s,NULL);
        g_object_unref(s);
    }

    s = g_socket_new(G_SOCKET_FAMILY_IPV6,
                            G_SOCKET_TYPE_STREAM,
                            G_SOCKET_PROTOCOL_TCP,
                            &e);
    if( e != NULL ){
        syslog(LOG_ERR,"interface_pushAddress: Error while creating socket: %s\n", e->message);
        g_error_free(e);
        return -1;
    }

    g_socket_connect(s, sa, NULL, &e);
    if( e != NULL ){
        syslog(LOG_ERR,"interface_pushAddress: Error while connecting: %s\n", e->message);
        g_error_free(e);
        return -1;
    }
    
    g_socket_send(s,"A",1,NULL,NULL);
    gchar *bytes = (gchar*)g_inet_address_to_bytes(n->netadr);
    g_socket_send(s,bytes,16,NULL,NULL);
    n->ubnetd = s;
    //gchar buf[1];
    //g_socket_receive(s,buf,1,NULL,NULL);
    return 0;
}

gint interface_removeAddress(struct node *n)
{
    GError * e = NULL;
    
    GInetAddress *lo = g_inet_address_new_loopback(
                            G_SOCKET_FAMILY_IPV6);
    GSocketAddress *sa = g_inet_socket_address_new(
                            lo,
                            42);
    
    GSocket *s = g_socket_new(G_SOCKET_FAMILY_IPV6,
                            G_SOCKET_TYPE_STREAM,
                            G_SOCKET_PROTOCOL_TCP,
                            &e);
    if( e != NULL ){
        syslog(LOG_ERR,"interface_popAddress: error while creating socket: %s\n", e->message);
        g_error_free(e);
        return -1;
    }

    g_socket_connect(s, sa, NULL, &e);
    if( e != NULL ){
        syslog(LOG_ERR,"interface_popAddress: error while connecting: %s\n", e->message);
        g_error_free(e);
        return -1;
    }
    
    g_socket_send(s,"D",1,NULL,NULL);
    gchar *bytes = (gchar*)g_inet_address_to_bytes(n->netadr);
    g_socket_send(s,bytes,16,NULL,NULL);
    gchar buf[1];
    g_socket_receive(s,buf,1,NULL,NULL);
    g_socket_close(s,NULL);
    g_object_unref(s);
    return 0;
}

GInetAddress *interface_getConfiguredAddress(struct node *n)
{
    GInetAddress *ret = NULL;
    GSocket *s = n->ubnetd;

    if( s != NULL ){
        if( g_socket_condition_check(s, G_IO_IN) & G_IO_IN ){
            gchar buf[1];
            g_socket_receive(s,buf,1,NULL,NULL);
            if( buf[0] == 'A' )
                ret = n->netadr;
            g_socket_close(s, NULL);
            g_object_unref(s);
            n->ubnetd = NULL;
        }
    }
    return ret;
}

