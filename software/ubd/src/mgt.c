#include <glib.h>
#include <gio/gio.h>
#include <stdio.h>
#include <string.h>

#include "mgt.h"
#include "address6.h"
#include "ubpacket.h"
#include "interface.h"

struct node nodes[MAX_NODE];

//static address_t mgt_getFreeAddress(void);
static struct node* mgt_getFreeNode(void);
static void mgt_checkTimeout(void);
static gboolean mgt_tick(gpointer data);

void mgt_init(void)
{
    gint i;
    for(i=0;i<MAX_NODE;i++){
        nodes[i].type=TYPE_NONE;
    }
    g_timeout_add_seconds(1,mgt_tick,NULL);
}

static void mgt_checkTimeout(void)
{
    gint i;
    for(i=4; i<MAX_NODE; i+=1){
        if( nodes[i].state == NODE_IDENTIFY ||
            nodes[i].state == NODE_NORMAL ){
            if( nodes[i].timeout-- == 0 ){
                printf("removing %s\n",nodes[i].id);
                net_removeSockets(&nodes[i]);
                address6_removeAddress(&nodes[i]);
                //TODO: remove sockets too
                nodes[i].type = TYPE_NONE;
            }
        }
    }
}

static gboolean mgt_tick(gpointer data)
{
    data = NULL;
    GInetAddress *addr;
    struct node *n;
    mgt_checkTimeout();
    while( (addr = interface_getConfiguredAddress()) != NULL ){
        if( (n = mgt_getNodeByNetAdr(addr)) != NULL ){
            net_createSockets(n);
        }
    }
    return TRUE;
}

struct node *mgt_createNode(gint type, gchar *id)
{
    struct node *n =  mgt_getFreeNode();
    if( n != NULL ){
        n->type = type;
        strncpy(n->id,id,MAX_ID);
        address6_createAddress(n);
        n->netup = FALSE;
    }
    return n;
}

struct node* mgt_getNodeById(gchar* id)
{
    gint i;
    for(i=0; i<MAX_NODE; i++){
        if( nodes[i].type != TYPE_NONE ){
            if( strncmp(id, nodes[i].id, MAX_ID) == 0){
                return &nodes[i];
            }
        }
    }
    printf("mgt: getnodebyid: node %s unknown\n",id);
    return NULL;
}

struct node* mgt_getNodeByBusAdr(gint adr)
{
    gint i;
    for(i=0; i<MAX_NODE; i++){
        if( nodes[i].type != TYPE_NONE ){
            if( nodes[i].busadr == adr ){
                return &nodes[i];
            }
        }
    }
//    printf("mgt: getnodebyname: node %s unknown\n",name);
    return NULL;
}

struct node* mgt_getNodeByNetAdr(GInetAddress *addr)
{
    gint i;
    for(i=0; i<MAX_NODE; i++){
        if( nodes[i].type != TYPE_NONE ){
            if( g_inet_address_get_native_size(addr) == 
                g_inet_address_get_native_size(nodes[i].netadr) &&
                memcmp(g_inet_address_to_bytes(addr),
                        g_inet_address_to_bytes(nodes[i].netadr),
                        g_inet_address_get_native_size(addr)) == 0 ){
                return &nodes[i];
            }
        }
    }
//    printf("mgt: getnodebyname: node %s unknown\n",name);
    return NULL;
}

/*address_t mgt_getFreeAddress(void)
{
    gint i;
    for(i=4; i<MAX_NODE; i+=1){
        if( nodes[i].type == TYPE_NONE){
            return i;
        }
    }
    return 0;
}*/

struct node* mgt_getFreeNode(void)
{
    gint i;
    for(i=4; i<MAX_NODE; i+=1){
        if( nodes[i].type == TYPE_NONE){
            nodes[i].busadr = i;       //FIXME fieser hack
            return &nodes[i];
        }
    }
    return NULL;
}

gint mgt_removeEntryById(gchar * id)
{
    id = NULL;
/*    GSequenceIter * i;
    for( i=g_sequence_get_begin_iter(entries);
            i!=g_sequence_get_end_iter(entries); 
            i=g_sequence_iter_next(i) ){
        struct entry *e = g_sequence_get(i);
        if( e->id != NULL && strcmp(id, e->id) == 0 ){
            g_free(e->id);
            g_object_unref(e->addr);
            g_sequence_remove(i);
            return 0;
        }
    }*/
    return 1;
}

