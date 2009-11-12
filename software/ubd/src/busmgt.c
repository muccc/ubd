#include <glib.h>
#include <stdio.h>
#include <string.h>

#include "packet.h"
#include "ubpacket.h"
#include "debug.h"
#include "busmgt.h"
#include "mgt.h"


void busmgt_init(void)
{
    //packet_addCallback(BUSMGT_ID, busmgt_inpacket);
    struct ubpacket reset;
    reset.dest = UB_ADDRESS_BROADCAST;
    reset.len = 1;
    reset.data[0] = 'r';
    packet_outpacket(&reset);
}

void busmgt_inpacket(struct ubpacket* p)
{
    //address_t new;
    gchar id[100];
    struct node * n;
    struct ubpacket response;

    printf("busmgt: read packet from %u to %u flags: %x len %u: ", 
            p->src, p->dest, p->flags, p->len);
    debug_hexdump(p->data, p->len);
    printf("\n");

    response.flags = UB_PACKET_MGT;
    switch(p->data[0]){
        //A new node tries to get an address
        case 'D':
            //TODO some len checks
            memcpy(id, p->data+1, p->len-1);
            id[p->len-1] = 0;

            printf("busmgt: got discover from %s\n", id);

            n = mgt_getNodeById(id);
            if( n == NULL ){
                printf("busmgt: creating new node\n");
                n = mgt_createNode(TYPE_NODE, id);
                if( n == NULL){
                    printf("busmgt: no free nodes available. please wait.\n");
                    return;
                }
                g_assert(n->busadr != 0);
                printf("busmgt: new address: %u\n",n->busadr);
            }else{
                printf("busmgt: old address: %u\n",n->busadr);
            }
            
            response.dest = UB_ADDRESS_BROADCAST;
            response.len = strlen(id)+3;
            response.data[0] = 'S';
            response.data[1] = n->busadr;
            strncpy((char*)response.data+2, id, UB_PACKET_DATA-2);
            
            if( response.len > UB_PACKET_DATA )
                response.len = UB_PACKET_DATA;
            response.data[response.len-1] = 0;

            packet_outpacket(&response);
            strcpy(n->id,id);
            //n->state = NODE_DISCOVER;
            n->state = NODE_IDENTIFY;
        break;
        
        //The node confirms an address
        case 'I':
            memcpy(id, p->data+1, p->len-1);
            id[p->len-1] = 0;
            printf("mgt: got identify from %s\n", id);

            n = mgt_getNodeById(id);
            if( n == NULL ){
                printf("Address %u unkown. Sending reset.\n", p->src);
                response.dest = p->src;
                response.len = 1;
                //response.data[0] = 'M';
                response.data[0] = 'r';
                packet_outpacket(&response);
                return;
            }
            //send the OK if we have our interface ready
            if( n->netadr != NULL ){
                response.dest = p->src;
                response.len = 1;
                response.data[0] = 'O';
                packet_outpacket(&response);
                //n->state = NODE_IDENTIFY;
                n->state = NODE_NORMAL;
                n->timeout = 30;
            }
        break;
        //a keep alive packet
        case 'A':
            n = mgt_getNodeByBusAdr(p->src);
            if( n == NULL ){
                printf("Address %u unkown. Sending reset.\n", p->src);
                response.dest = p->src;
                response.len = 1;
                //response.data[0] = 'M';
                response.data[0] = 'r';
                packet_outpacket(&response);
            }else{
                //reset the timeout
                n->timeout = 30;
                /*response.dest = p->src;
                response.len = 1;
                response.flags ^= UB_PACKET_MGT;
                response.data[0] = 'V';
                packet_outpacket(&response);*/
            }
        break;
    };
    g_free(p); 
}


