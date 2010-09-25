#include <glib.h>
#include <stdio.h>
#include <string.h>

#include "packet.h"
#include "ubpacket.h"
#include "debug.h"
#include "busmgt.h"
#include "mgt.h"
#include "ubconfig.h"

static void busmgt_sendReset(uint8_t adr);
static void busmgt_setQueryInterval(uint8_t adr, uint16_t interval);
static void busmgt_sendCmd(uint8_t adr, uint8_t cmd);
static void busmgt_setAddress(uint8_t adr, gchar *id);
static void busmgt_sendOK(uint8_t adr);

void busmgt_init(void)
{
    busmgt_sendReset(UB_ADDRESS_BROADCAST);
}

gint busmgt_getFreeBusAdr()
{
    int i;
    for(i=3; i<255; i++){
        if( nodes_getNodeByBusAdr(i) == NULL )
            return i;
    }
    printf("busmgt.c: warning: found no free address!\n");
    return -1;
}

void busmgt_inpacket(struct ubpacket* p)
{
    //TODO: check len of id buffer
    gchar id[100];
    struct node * n;
    uint16_t interval;


    switch(p->data[0]){
        //A new node tries to get an address
        case 'D':
            //TODO some len checks
            interval = (p->data[1]<<8) + p->data[2];
            memcpy(id, p->data+3, p->len-3);
            id[p->len-3] = 0;

            printf("busmgt: got discover from %s\n", id);

            //n = nodes_getNodeById(id);
            //if( n == NULL ){
                printf("busmgt: creating new node\n");
                n = mgt_createNode(TYPE_NODE, id);
                if( n == NULL){
                    printf("busmgt: no free nodes available. please wait.\n");
                    return;
                }
                g_assert(n->busadr != 0);
                printf("busmgt: new bus address: %u\n",n->busadr);
            //}else{
            //    printf("busmgt: old bus address: %u\n",n->busadr);
            //}

            //TODO: check if it is possible for the bus to query
            //the node at this interval
            busmgt_setQueryInterval(n->busadr, interval);
            busmgt_setAddress(n->busadr, id);
            n->state = NODE_IDENTIFY;
            n->timeout = 30;
        break;
        
        //The node confirms an address
        case 'I':
            //TODO:check len
            memcpy(id, p->data+1, p->len-1);
            id[p->len-1] = 0;
            printf("busmgt: got identify from %s\n", id);

            n = nodes_getNodeById(id);
            if( n == NULL ){
                printf("busmgt: unkown node\n");
                busmgt_sendReset(p->src);
                return;
            }
            //send the OK if we have our interface ready
            if( n->netup == TRUE){
                printf("busmgt: node known\n");
                busmgt_sendOK(p->src);
                //n->state = NODE_IDENTIFY;
                n->state = NODE_NORMAL;
                n->timeout = 30;
                n->busup = TRUE;
                nodes_activateNode(n);
            }

        break;
        case 'V':
            n = nodes_getNodeByBusAdr(p->src);
            memcpy(n->version, p->data+2, p->len-2);
        break;
        //a keep alive packet
        case 'A':
            n = nodes_getNodeByBusAdr(p->src);
            if( n == NULL ){
                busmgt_sendReset(p->src);
                return;
            }
            //reset the timeout
            n->timeout = 50;
        break;
        //the bridge id
        case 'B':
            memcpy(id, p->data+1, p->len-1);
            id[p->len-1] = 0;
            printf("busmgt: bridge id %s\n", id);
            mgt_createBridge(id);
            n = nodes_getNodeByBusAdr(p->src);
            busmgt_sendOK(p->src);
            n->state = NODE_NORMAL;
            n->timeout = 30;
            n->busup = TRUE;

        break;
    };
}

//add an address to a multicast group
void busmgt_addToMulticast(uint8_t adr, uint8_t mcast)
{
    printf("Adding node %d to multicast group %d\n",adr,mcast);
    uint8_t data[] = {mcast};
    busmgt_sendCmdData(adr,'1',data,sizeof(data));
}

void busmgt_setName(uint8_t adr, char *name)
{
    printf("Setting name of %u to %s\n",adr,name);
    //append the trailing 0
    busmgt_sendCmdData(adr,'s',(uint8_t*)name,strlen(name)+1);
}

static void busmgt_sendReset(uint8_t adr)
{
    printf("Setting reset to %u\n", adr);
    //busmgt_sendCmd(adr,'r');
    struct ubpacket p;
    p.dest = adr;
    p.len = 1;
    p.data[0] = 'r';
    //don't expect an ack for a reset
    p.flags = UB_PACKET_MGT | UB_PACKET_NOACK;
    packet_outpacket(&p);}

static void busmgt_setQueryInterval(uint8_t adr, uint16_t interval)
{
    printf("Setting query interval of %u to %u\n", adr,interval);
    uint8_t data[] = {adr,interval>>8,interval&0xFF};
    busmgt_sendCmdData(UB_ADDRESS_BRIDGE,'q',data,sizeof(data));
}

//send a broadcast to set the address of the node with name '*id'
static void busmgt_setAddress(uint8_t adr, gchar *id)
{
    uint8_t data[strlen(id)+2];
    data[0] = adr;
    strcpy((char*)data+1, id);
    busmgt_sendCmdData(UB_ADDRESS_BROADCAST,'S',data,sizeof(data));
}

//send an OK to the address and request the software version
static void busmgt_sendOK(uint8_t adr)
{
    printf("Sending OK to %u\n",adr);
    busmgt_sendCmd(adr,'O');

    printf("Getting version from  %u\n",adr);
    busmgt_sendCmd(adr,'V');

    printf("sending multicast groups\n");
    busmgt_sendCmdData(adr,'A',(uint8_t*)"\xFE",1);
}

//send a command without any data
static void busmgt_sendCmd(uint8_t adr, uint8_t cmd)
{
    busmgt_sendCmdData(adr,cmd,NULL,0);
}

//send a command with attaches data
void busmgt_sendCmdData(uint8_t adr, uint8_t cmd,
                                uint8_t *data, uint8_t len)
{
    struct ubpacket p;
    p.dest = adr;
    p.len = len+1;
    p.data[0] = cmd;
    //g_assert(p.len <= UB_PACKET_DATA);
    if( p.len > sizeof(p.data)){
        printf("busmgt_sendCmdData(): packet to big!\n");
        //TODO: log this error
        //TODO: fragment packet?
        return;
    }

    while(len--)
        p.data[len+1] = data[len];

    p.flags = UB_PACKET_MGT;

    packet_outpacket(&p);
}

void busmgt_sendData(uint8_t adr, uint8_t *data, uint8_t len)
{
    struct ubpacket p;
    p.dest = adr;
    p.len = len;
    //g_assert(p.len <= UB_PACKET_DATA);
    if( p.len > sizeof(p.data)){
        printf("busmgt_sendCmdData(): packet to big!\n");
        //TODO: log this error
        //TODO: fragment packet?
        return;
    }

    while(len--)
        p.data[len] = data[len];

    p.flags = UB_PACKET_MGT;

    packet_outpacket(&p);
}

