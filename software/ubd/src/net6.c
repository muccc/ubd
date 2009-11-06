#include <gnet.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gio/gio.h>

#include "debug.h"

struct entry {
    gchar * id;
    GInetAddr*  addr;
    GUdpSocket* udp;
};
GSequence * entries;

GIOChannel * udpio;
GInetAddr*  net_base;
GInetAddr*  net_last;
GUdpSocket*  udpsocket;
gint        net_prefix;
char        net_interface[1024];

gboolean udp_read(GIOChannel* serial, GIOCondition condition, gpointer data)
{
    g_file_new_for_path("blubb");
    uint8_t buf[100];
    gsize len;
    GInetAddr * src;
    serial = NULL;
    condition = 0;
    data = NULL;
    len = gnet_udp_socket_receive(udpsocket, (char*)buf, 100, &src);
    if( len ){
        printf("udp: received:");debug_hexdump(buf,len);printf("\n");
    }else{
        printf("udp: received empty msg\n");
    }
    return TRUE;
}

gboolean entry_udp_read(GIOChannel* iochan, GIOCondition condition, gpointer data)
{
    uint8_t buf[100];
    GInetAddr * src;
    struct entry *e = data;
    iochan = NULL;
    condition = 0;

    int len = gnet_udp_socket_receive(e->udp, (char*)buf, 100, &src);
    if( len ){
        printf("udp: received:");debug_hexdump(buf,len);printf("\n");
        bus_sendToID(e->id, buf, len, FALSE);
        gnet_udp_socket_send(e->udp, "ACK", 3, src);
    }else{
        printf("udp: received empty msg\n");
    }
    return TRUE;
}

gint net_init(gchar* interface, gchar* baseaddress, gint prefix)
{
    gnet_init();
    gnet_ipv6_set_policy(GIPV6_POLICY_IPV6_ONLY);
    entries = g_sequence_new(g_free);

    net_prefix = prefix;

    strcpy(net_interface, interface);
    if( !gnet_inetaddr_is_canonical(baseaddress) ){
        printf("base address is not canonical\n");
        return -1;
    }

    net_base = gnet_inetaddr_new_nonblock(baseaddress, 0);
    if( net_base == NULL ){
        printf("base address not available\n");
        return -1;
    }
    
    net_last = gnet_inetaddr_clone(net_base);

    udpsocket = gnet_udp_socket_new_full(net_base, 2323);
    if( udpsocket == NULL ){
        printf("gnet_udp_socket_new_full failed\n");
        return -1;
    }

    udpio = gnet_udp_socket_get_io_channel(udpsocket);
    g_io_add_watch(udpio,G_IO_IN,udp_read,udpsocket);
    return 0;
}

void net_addEntry(gchar *id, GInetAddr *addr)
{
    struct entry *e = g_new(struct entry,1);
    e->id = id;
    e->addr = addr;
    
    gchar *tmp = gnet_inetaddr_get_canonical_name(addr);
    printf("creating udp socket on ip: %s\n",tmp);
    g_free(tmp);

    e->udp = gnet_udp_socket_new_full(addr, 2323);
    g_assert(e->udp != NULL);
    g_io_add_watch(
        gnet_udp_socket_get_io_channel(e->udp),
        G_IO_IN,
        entry_udp_read,
        e);
    g_sequence_append(entries,e);
}

struct entry * net_getEntryById(gchar * id)
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
}

gint net_removeEntryById(gchar * id)
{
    GSequenceIter * i;
    for( i=g_sequence_get_begin_iter(entries);
            i!=g_sequence_get_end_iter(entries); 
            i=g_sequence_iter_next(i) ){
        struct entry *e = g_sequence_get(i);
        if( e->id != NULL && strcmp(id, e->id) == 0 ){
            g_free(e->id);
            g_free(e->addr);
            g_sequence_remove(i);
            return 0;
        }
    }
    return 1;
}


//TODO: add something real here
GInetAddr* net_getFreeAddr(void)
{
    gchar buf[16];
    //gchar * tmp = gnet_inetaddr_get_canonical_name(net_last);
    gnet_inetaddr_get_bytes(net_last,buf);
    //printf("last address: %s\n",tmp);
    //g_free(tmp);
    
    buf[15]++;
    gnet_inetaddr_set_bytes(net_last,buf,16);
    GInetAddr*  addr = gnet_inetaddr_new_bytes(buf,16);

    gchar *tmp = gnet_inetaddr_get_canonical_name(net_last);
    printf("free address: %s\n",tmp);
    g_free(tmp);
    return addr;
}

gint net_createAddress(GInetAddr*  addr)
{
    char buf[1024];
    char *tmp = gnet_inetaddr_get_canonical_name(addr);

    sprintf(buf,"ip addr del %s dev %s",
                            tmp, net_interface);

    system(buf);
    
    sprintf(buf,"ip addr add %s dev %s",
                            tmp, net_interface);

    int rc = system(buf);

    g_free(tmp);
    if( rc ){
        printf("return value: %d\n",rc);
        return -1;
    }
    return 0;

}

void net_removeAddress(GInetAddr*  addr)
{
    char buf[1024];
    char *tmp = gnet_inetaddr_get_canonical_name(addr);

    sprintf(buf,"ip addr del %s dev %s",
                            tmp, net_interface);

    system(buf);
    
    g_free(tmp);
    return;
}

/*
 * Tries to add a new address to the interface supplied to net_init.
 * If there is already an address for this ID the no new address will
 * be created.
 * Parameter: id
 * Returns: 0 on success, -1 on error
*/

gint net_addAddressForID(gchar * id)
{
    printf("adding address for %s\n",id);

    if( net_getEntryById(id) != NULL ){
        printf("entry already exists\n");
        return 0;
    }
    
    GInetAddr *addr = net_getFreeAddr();
    if( net_createAddress(addr) == -1 )
        return -1;
    net_addEntry(g_strdup(id), addr);
    return 0;
}


gint net_removeAddressForID(gchar * id)
{
    struct entry *e;
    if( (e = net_getEntryById(id)) != NULL ){
        net_removeAddress(e->addr);
        return net_removeEntryById(id);
    }
    return -1;
}
