#include <stdio.h>
#include <glib.h>
#include <gio/gio.h>
#include "address6db.h"
#include "nodes.h"

GInetAddress *address6db_last;

void address6db_init(GInetAddress *base)
{
    address6db_last = g_inet_address_new_from_bytes(
                    g_inet_address_to_bytes(base),
                    G_SOCKET_FAMILY_IPV6);

}

gboolean db_isIPKnown(GInetAddress *addr)
{
    int i;
    gchar *tmp = g_inet_address_to_string(addr);
    gint count = nodes_getNodeCount();
    for(i=0;i<count;i++){
        struct node *n = nodes_getNode(i);
        printf("checking node %d\n",i);
        if( n->netadr != NULL ){
            //printf("found used ip\n");
            if( memcmp( g_inet_address_to_bytes(n->netadr),
                g_inet_address_to_bytes(addr),16) == 0 ){
                printf("ip %s used by node %s\n",tmp,n->id);
                g_free(tmp);
                return TRUE;
            }
        }
    }
    printf("ip %s unknown\n",tmp);
    g_free(tmp);
    return FALSE;
}

//TODO: use a better algorithm to calculate new addresse
//based on the netmask
GInetAddress *address6db_getFreeAddr(gchar *id)
{
    gchar *tmp;
    guint8 *buf;
    GInetAddress *addr;
    id = NULL;
    do{
        //tmp = g_inet_address_to_string(address6db_last);
        //printf("old address: %s\n",tmp);
        //g_free(tmp);
        
        buf = (guint8*) g_inet_address_to_bytes(address6db_last);
        buf[15]++; 
        
        addr = g_inet_address_new_from_bytes(buf,G_SOCKET_FAMILY_IPV6);
        buf = (guint8*) g_inet_address_to_bytes(addr);
        
        g_object_unref(address6db_last);
        address6db_last = g_inet_address_new_from_bytes(buf,G_SOCKET_FAMILY_IPV6);
        
        //tmp = g_inet_address_to_string(addr);
        //printf("checking address: %s\n",tmp);
        //g_free(tmp);
    }while( db_isIPKnown(addr) == TRUE );
    tmp = g_inet_address_to_string(addr);
    printf("using address: %s\n",tmp);
    g_free(tmp);
    return addr;
}

