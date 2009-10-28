#include <glib.h>
#include <stdio.h>
#include <string.h>

#include "packet.h"
#include "ubpacket.h"
#include "debug.h"
#include "busmgt.h"

enum nodestate{
    NODE_UNKNOWN,
    NODE_TIMEOUT,
    NODE_DISCOVER,
    NODE_IDENTIFY,
    NODE_NORMAL
};

#define MAX_NAME    100
struct node{
    address_t   adr;
    gint        state;
    gint        tpoll;
    gint        poll;
    gchar       name[MAX_NAME];
    gint        timeout;
    gint        ttimeout;
};

#define MAX_NODE    256
struct node nodes[MAX_NODE];

struct node* busmgt_getNodeByName(gchar* name)
{
    gint i;
    for(i=4; i<MAX_NODE; i++){
        if( nodes[i].state != NODE_UNKNOWN ){
            if( strncmp(name, nodes[i].name, MAX_NAME) == 0){
                return &nodes[i];
            }
        }
    }
    printf("mgt: getnodebyname: node %s unknown\n",name);
    return NULL;
}

struct node* busmgt_getNodeByAdr(address_t adr)
{
    gint i;
    for(i=4; i<MAX_NODE; i+=10){
        if( nodes[i].state != NODE_UNKNOWN ){
            if( nodes[i].adr == adr ){
                return &nodes[i];
            }
        }
    }
//    printf("mgt: getnodebyname: node %s unknown\n",name);
    return NULL;
}

address_t busmgt_getFreeAddress(void)
{
    gint i;
    for(i=4; i<MAX_NODE; i+=10){
        if( nodes[i].state == NODE_UNKNOWN){
            return i;
        }
    }
    return 0;
}

struct node* busmgt_getFreeNode(void)
{
    gint i;
    for(i=4; i<MAX_NODE; i+=10){
        if( nodes[i].state == NODE_UNKNOWN){
            nodes[i].adr = i;       //FIXME fieser hack
            return &nodes[i];
        }
    }
    return NULL;
}

void busmgt_inpacket(struct ubpacket* p)
{
    //address_t new;
    gchar name[100];
    struct node * n;
    struct ubpacket response;

    printf("mgt: read packet from %u to %u flags: %x len %u: ", 
            p->src, p->dest, p->flags, p->len);
    debug_hexdump(p->data, p->len);
    printf("\n");

    switch(p->data[1]){
        case 'D':
            //memcopy(name, p->data+2);//     TODO some checks, p->len-1);
            memcpy(name, p->data+2, p->len-2);
            name[p->len-2] = 0;
            n = busmgt_getNodeByName(name);
            
            printf("mgt: got discover from %s\n", name);

            if( n == NULL ){
                //new = busmgt_getFreeAddress();
                n = busmgt_getFreeNode();
                if( n == NULL){
                    printf("mgt: no free node available. please wait.\n");
                    return;
                }
                printf("mgt: new address: %u\n",n->adr);
            }else{
                printf("mgt: old address: %u\n",n->adr);
            }
            
            if( n->adr == 0 ){
                printf("mgt: adr was 0?\n");
                return;
            }

            response.dest = UB_ADDRESS_BROADCAST;
            response.len = strlen(name)+4;
            response.data[0] = 'M';
            response.data[1] = 'S';
            response.data[2] = n->adr;
            strncpy((char*)response.data+3, name, UB_PACKET_DATA-3);
            
            if( response.len > UB_PACKET_DATA )
                response.len = UB_PACKET_DATA;
            response.data[response.len-1] = 0;

            packet_outpacket(&response);
            strcpy(n->name,name);
            n->state = NODE_DISCOVER;

        break;
        case 'I':
//            strncpy(name, p->data+2, p->len-1);
            memcpy(name, p->data+2, p->len-2);
            name[p->len-2] = 0;

            printf("mgt: got identify from %s\n", name);
            n = busmgt_getNodeByName(name);

            if( n == NULL ){
                printf("Address %u unkown. Sending reset.\n", p->src);
                response.dest = p->src;
                response.len = 1;
                //response.data[0] = 'M';
                response.data[0] = 'r';
                packet_outpacket(&response);
                return;
            }
            
            response.dest = p->src;
            response.len = 2;
            response.data[0] = 'M';
            response.data[1] = 'O';
            packet_outpacket(&response);
            n->state = NODE_IDENTIFY;
        break;
        case 'A':
            n = busmgt_getNodeByAdr(p->src);
            if( n == NULL ){
                printf("Address %u unkown. Sending reset.\n", p->src);
                response.dest = p->src;
                response.len = 1;
                //response.data[0] = 'M';
                response.data[0] = 'r';
                packet_outpacket(&response);
            }else{
                n->timeout = 0;
                n->state = NODE_NORMAL;
                response.dest = p->src;
                response.len = 1;
                //response.data[0] = 'M';
                response.data[0] = 'V';
                packet_outpacket(&response);

            }
        break;
    };

    g_free(p); 
}

void busmgt_init(void)
{

    struct ubpacket response;
    gint i;
    for(i=0;i<256;i++){
        nodes[i].state=NODE_UNKNOWN;
    }
    packet_addCallback(BUSMGT_ID, busmgt_inpacket);
                response.dest = 0xFF;
                response.len = 1;
                response.data[0] = 'r';
                packet_outpacket(&response);

}
