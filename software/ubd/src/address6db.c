#include <stdio.h>
#include <glib.h>
#include <gio/gio.h>
#include "address6db.h"
#include "db.h"

GInetAddress *address6db_last;

void address6db_init(GInetAddress *base)
{
    address6db_last = g_inet_address_new_from_bytes(
                    g_inet_address_to_bytes(base),
                    G_SOCKET_FAMILY_IPV6);

}

//TODO: add something real here
GInetAddress *address6db_getFreeAddr(gchar *id)
{
    gchar *tmp;
    guint8 *buf;
    GInetAddress *addr;
    do{
        tmp = g_inet_address_to_string(address6db_last);
        printf("old address: %s\n",tmp);
        g_free(tmp);
        
        buf = (guint8*) g_inet_address_to_bytes(address6db_last);
        buf[15]++; 
        
        addr = g_inet_address_new_from_bytes(buf,G_SOCKET_FAMILY_IPV6);
        buf = (guint8*) g_inet_address_to_bytes(addr);
        
        g_object_unref(address6db_last);
        address6db_last = g_inet_address_new_from_bytes(buf,G_SOCKET_FAMILY_IPV6);
        
        tmp = g_inet_address_to_string(addr);
        printf("free address: %s\n",tmp);
        g_free(tmp);
    }while( db_isIPKnown(addr) == TRUE );
    db_addNode(id, addr);
    return addr;
}

