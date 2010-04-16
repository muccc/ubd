#include <glib.h>
#include <gio/gio.h>
#include <stdio.h>
#include <string.h>

#include "mgt.h"
#include "address6.h"
#include "ubpacket.h"
#include "interface.h"
#include "db.h"

struct node nodes[MAX_NODE];

static struct node* mgt_getFreeNode(void);
static void mgt_checkTimeout(void);
static gboolean mgt_tick(gpointer data);
static void mgt_setNameFromID(struct node *n);
static void mgt_registerNode(struct node * n, char *id,
                             uint8_t type, uint8_t busadr);

void mgt_init(void)
{
    gint i;

    for(i=0;i<MAX_NODE;i++){
        nodes[i].type=TYPE_NONE;
    }

    nodes[2].type = TYPE_BRIDGE;  //this node is reserved
    nodes[2].netadr = NULL;       //this node is reserved
    g_timeout_add_seconds(1,mgt_tick,NULL);
}

static gboolean mgt_tick(gpointer data)
{
    data = NULL;
    GInetAddress *addr;
    struct node *n;
    mgt_checkTimeout();
    while( (addr = interface_getConfiguredAddress()) != NULL ){
        g_assert( (n = mgt_getNodeByNetAdr(addr)) != NULL );
        net_createSockets(n);
    }
    return TRUE;
}

struct node *mgt_createNode(gint type, gchar *id)
{
    struct node *n =  mgt_getFreeNode();
    if( n != NULL ){
        mgt_registerNode(n, id, type, 0);
    }
    return n;
}

struct node *mgt_createBridge(gchar *id)
{
    struct node *n = &nodes[2];
    mgt_registerNode(n, id, TYPE_BRIDGE, 2);
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
    return NULL;
}

struct node* mgt_getNodeByNetAdr(GInetAddress *addr)
{
    gint i;
    for(i=0; i<MAX_NODE; i++){
        if( nodes[i].type != TYPE_NONE && nodes[i].netadr != NULL ){
            if( g_inet_address_get_native_size(addr) == 
                g_inet_address_get_native_size(nodes[i].netadr) &&
                memcmp(g_inet_address_to_bytes(addr),
                        g_inet_address_to_bytes(nodes[i].netadr),
                        g_inet_address_get_native_size(addr)) == 0 ){
                return &nodes[i];
            }
        }
    }
    return NULL;
}

static void mgt_registerNode(struct node * n, char *id, uint8_t type, uint8_t busadr)
{
    n->type = type;
    if( busadr )
        n->busadr = busadr;
    strncpy(n->id,id,MAX_ID);

    if( db_isNodeKnown(n->id) ){
        printf("node known\n");
        db_loadNode(n);
        address6_createAddress(n);
    }else{
        printf("node unknown\n");
        n->netup = FALSE;
        memset(n->groups,0,sizeof(n->groups));
        n->netadr = NULL;
        mgt_setNameFromID(n);
        address6_createAddress(n);
        db_addNode(n);
    }
}

//Create the name of the node
//If the id contains a ',' marking the start of the domain
//the domain is ommitted
static void mgt_setNameFromID(struct node *n)
{
    //n->name is of size n->id
    g_assert(sizeof(n->name) >= sizeof(n->id));
    strcpy(n->name,n->id);
    char *s = strchr(n->name,',');

    if( s != NULL){
        *s = 0;
        //TODO: make sure n->domain is large enough
        g_assert(sizeof(n->domain) >= sizeof(n->id));
        strcpy(n->domain,s++);
    }else{
        //there was no domain in the id
        printf("ill formated id for this node: %s\n",n->id);
    }
}

static struct node* mgt_getFreeNode(void)
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



/*gint mgt_removeEntryById(gchar * id)
{
    id = NULL;
    GSequenceIter * i;
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
    }
    return 1;
}*/
