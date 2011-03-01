#include <stdio.h>
#include <glib.h>
#include <gio/gio.h>
#include "address6db.h"
#include "nodes.h"

//GInetAddress *address6db_last;
GInetAddress *baseaddress;

void address6db_init(GInetAddress *base)
{
//    address6db_last = g_inet_address_new_from_bytes(
//                    g_inet_address_to_bytes(base),
//                    G_SOCKET_FAMILY_IPV6);
    baseaddress = g_inet_address_new_from_bytes(
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
    guint8 addrbuf[16];
    GInetAddress *addr;

    memcpy(addrbuf,(guint8*) g_inet_address_to_bytes(baseaddress),16);
    
    GChecksum* checksum = g_checksum_new(G_CHECKSUM_MD5);
    g_checksum_update(checksum, (guchar*)id, strlen(id));

    guint8 digest[128];
    gsize size = 128;

    g_checksum_get_digest(checksum, digest, &size);
    g_checksum_free(checksum);

    memcpy(addrbuf+8,digest,8);

    addr = g_inet_address_new_from_bytes(addrbuf,G_SOCKET_FAMILY_IPV6);
    
    gchar *tmp = g_inet_address_to_string(addr);
    printf("using address: %s\n",tmp);
    g_free(tmp);
    return addr;
}

