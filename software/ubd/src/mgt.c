#include <glib.h>
#include <gio/gio.h>
#include <stdio.h>
#include <string.h>

#include "mgt.h"
#include "busmgt.h"
#include "address6.h"
#include "ubpacket.h"
#include "interface.h"
#include "nodes.h"

static void mgt_checkTimeout(void);
static gboolean mgt_tick(gpointer data);
static struct node *mgt_registerNode(char *id,
                             uint8_t type, uint8_t busadr);

void mgt_init(void)
{
    g_timeout_add_seconds(1,mgt_tick,NULL);
}

static gboolean mgt_tick(gpointer data)
{
    data = NULL;
    mgt_checkTimeout();
    
    gint count = nodes_getNodeCount();
    gint i;
    for(i=0; i<count; i++){
        struct node *n = nodes_getNode(i);
        if( interface_getConfiguredAddress(n) != NULL )
            net_createSockets(n);
    }
    return TRUE;
}

struct node *mgt_createNode(gint type, gchar *id)
{
    return mgt_registerNode(id, type, 0);
}

struct node *mgt_createBridge(gchar *id)
{
    return mgt_registerNode(id, TYPE_BRIDGE, 2);
}

static struct node *mgt_registerNode(char *id, uint8_t type, uint8_t busadr)
{
    struct node *dbn = nodes_getNodeById(id);
    
    if( dbn != NULL){
        printf("node known\n");
    }else{
        printf("node unknown\n");
        dbn = nodes_getFreeNode();
        if( dbn == NULL ){
            printf("mgt.c: warning: got no new node!\n");
            return dbn;     //FIXME don't fail silently
        }
        printf("copying id %s\n",id);
        strncpy(dbn->id,id,MAX_ID);
        printf("adding node %s\n",dbn->id);
        nodes_addNode(dbn);
    }
    if( busadr ){
        dbn->busadr = busadr;
        printf("using fixed bus addr %d\n",busadr);
    }else{
        dbn->busadr = busmgt_getFreeBusAdr();
    }
    dbn->type = type;
    address6_createAddress(dbn);
    printf("activating node\n");
    nodes_activateNode(dbn);
    return dbn;
}

static void mgt_checkTimeout(void)
{
    gint count = nodes_getNodeCount();
    gint i;
    for(i=0; i<count; i++){
        struct node *n = nodes_getNode(i);
        if( n->state == NODE_IDENTIFY || n->state == NODE_NORMAL ){
            if( n->timeout-- == 0 ){
                printf("removing %s\n",n->id);
                net_removeSockets(n);
                address6_removeAddress(n);
                n->type = TYPE_NONE;
                nodes_deactivateNode(n);
            }
        }
    }
}

