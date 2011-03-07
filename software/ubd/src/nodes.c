#include <glib.h>
#include "nodes.h"
#include "mgt.h"

struct node nodes[MAX_NODE];

void nodes_init(void)
{
    gint i;
    for(i=0;i<MAX_NODE;i++){
        nodes[i].free=TRUE;
        nodes[i].type=TYPE_NONE;
        nodes[i].id[0] = 0;
        nodes[i].name[0] = 0;
        nodes[i].domain[0] = 0;
        nodes[i].version[0] = 0;
        nodes[i].netadr = NULL;
        nodes[i].ubnetd = NULL;
        nodes[i].avahiaddressgroup = NULL;
        gint j;
        for(j=0; j<32; j++){
            nodes[i].tcpsockets[j].listeners = NULL;
            nodes[i].tcpsockets[j].avahiservicegroup = NULL;
            nodes[i].udpsockets[j].avahiservicegroup = NULL;
        }

    }
}

struct node *nodes_getFreeNode(void)
{
    gint i,j;
    for(i=0;i<MAX_NODE;i++){
        if( nodes[i].free == TRUE ){
            nodes[i].netup = FALSE;
            for(j=0; j<32; j++)
                nodes[i].groups[j] = -1;
            memset(nodes[i].classes,0,sizeof(nodes[i].classes));
            nodes[i].netadr = NULL;
            nodes[i].avahiaddressgroup = NULL;
            guint j;
            for(j=0; j<32; j++){
                nodes[i].tcpsockets[j].listeners = NULL;
                nodes[i].tcpsockets[j].avahiservicegroup = NULL;
                nodes[i].udpsockets[j].avahiservicegroup = NULL;
            }
            return &nodes[i];
        }
    }
    printf("nodes.c: warning: no free node found!\n");
    return NULL;
}

void nodes_addNode(struct node *newnode)
{
    //just assume that the node is already in our list
    //and mark it as in use
    printf("adding node %s to list\n",
                            newnode->id);
    newnode->free = FALSE;
    nodes_setNameFromID(newnode);
}

gint nodes_getNodeCount(void)
{
    gint i;
    gint count = 0;
    //TODO: maybe get rid of the loop for performance
    for(i=0;i<MAX_NODE;i++){
        if( nodes[i].free == FALSE )
            count++;
    }
    return count;
}

struct node *nodes_getNode(gint node)
{
    gint i;
    for(i=0;i<MAX_NODE;i++){
        if( nodes[i].free == FALSE ){
            if( node-- == 0 ){
                return &nodes[i];
            }
        }
    }
    return NULL;
}

struct node *nodes_getNodeById(gchar *id)
{
    gint i;
    for(i=0; i<MAX_NODE; i++){
        if( nodes[i].free == FALSE && 
            strcmp(id,nodes[i].id) == 0 ){
            return &nodes[i];
        }
    }

    printf("nodes.c: warning: getnodebyid: node %s not found\n",id);
    return NULL;
}

struct node *nodes_getNodeByBusAdr(gint adr)
{
    gint i;
    for(i=0; i<MAX_NODE; i++){
        if( nodes[i].free == FALSE && 
            nodes[i].active == TRUE &&
            nodes[i].busadr == adr ){
                return &nodes[i];
        }
    }

    printf("nodes.c: warning: getnodebybusadr: node %d not found\n",
            adr);
    return NULL;
}

struct node *nodes_getNodeByNetAdr(GInetAddress *addr)
{
    gint i;
    for(i=0; i<MAX_NODE; i++){
        if( nodes[i].free == FALSE && nodes[i].netadr != NULL ){
            if( g_inet_address_get_native_size(addr) == 
                g_inet_address_get_native_size(nodes[i].netadr) &&
                memcmp(g_inet_address_to_bytes(addr),
                        g_inet_address_to_bytes(nodes[i].netadr),
                        g_inet_address_get_native_size(addr)) == 0 ){
                return &nodes[i];
            }
        }
    }

    printf("nodes.c: warning: getnodebynetadr: node not found\n");
    return NULL;
}

void nodes_activateNode(struct node *node)
{
    g_assert(node->free == FALSE);
    //printf("nodes.c: warning: trying to activate a free node!\n");
    node->active = TRUE;
}

void nodes_deactivateNode(struct node *node)
{
    g_assert(node->free == FALSE);
    //    printf("nodes.c: warning: trying to");
    //    printf("deactivate a free node!\n");
    node->active = FALSE;
}

//Create the name of the node
//If the id contains a ',' marking the start of the domain
//the domain is ommitted
void nodes_setNameFromID(struct node *n)
{
    //n->name is of size n->id
    g_assert(sizeof(n->name) >= sizeof(n->id));
    strcpy(n->name,n->id);
    char *s = strchr(n->name,',');

    if( s != NULL){
        *s = 0;
        g_assert(sizeof(n->domain) >= sizeof(n->id));
        strcpy(n->domain,++s);
    }else{
        //there was no domain in the id
        printf("ill formated id for this node: %s\n",n->id);
        n->domain[0] = 0;
    }

    //TODO: check buffer
    strcpy(n->hostname, n->name);
    strcat(n->hostname, ".local");

}

