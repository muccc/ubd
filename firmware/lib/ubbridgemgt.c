//#include <avr/io.h>
#include <string.h>
#include <stdio.h>

#include "ub.h"
#include "ubpacket.h"
#include "ubbridgemgt.h"
#include "ubaddress.h"

#include "settings.h"
#include "ubstat.h"
#include "ubrs485master.h"

#include "udebug.h"

#define  MGT_BRIDGEDISCOVER     'B'
#define  MGT_BRIDGEALIVE        'A'

uint8_t ubbridgemgt_state;

enum slavemgtstate {
    DISCOVER,
    IDENTIFY,
    CONNECTED
};

void ubbridgemgt_init(void)
{
    ubbridgemgt_state = DISCOVER;
}

uint8_t ubbridgemgt_process(struct ubpacket_t * p)
{
    uint8_t * data = p->data;
    struct ubstat_t * flags;
    struct ubpacket_t * out;
    if(!(p->header.flags & UB_PACKET_MGT))
        return 0;
    switch(data[0]){
        case 'q':
            //TODO: check if it's on the rf
            flags = ubstat_getFlags(data[1]);
            //if( flags->rs485 ){
                rs485master_setQueryInterval(data[1], (data[2] << 8) + data[3]);
                flags->known = 1;
            //}
            //flags->rs485 = 1;
        break;
        case 'O':
            ubconfig.configured = 1;
            ubbridgemgt_state = CONNECTED;
            UDEBUG("Dconfigured");
        break;
        case 's':
            ubadr_setID(data+1);
        break;
        case 'r':
            //XXX: think about implementing this again
            //why did this get removed?
            //we don't want to get reset when the host resets
            //the whole bus
            //while(1);
        break;
        case 'V':
            out = ubpacket_getSendBuffer();
            out->header.dest = UB_ADDRESS_MASTER;
            out->header.src = ubadr_getAddress();
            out->header.flags = UB_PACKET_MGT;
            sprintf((char *)out->data,"V="__DATE__);
            out->header.len = strlen((char*)out->data);
            return 1;
        break;
        case 'A':
            ubadr_addMulticast(data[1]);
        break;
        case 'R':
            ubadr_removeMulticast(data[1]);
        break;
    }
    return 0;
}

void ubbridgemgt_tick(void)
{
    struct ubpacket_t * p;
    static uint16_t time = 0;
    static uint8_t flags = 0;
    if(!time--){
        time = 5000;
        switch(ubbridgemgt_state){
            case DISCOVER:
                flags |= 0x01;
            break;
            case CONNECTED:
                flags |= 0x02;
            break;
        }
    }

    if(flags && ubpacket_free()){
        p = ubpacket_getSendBuffer();
        p->header.src = ubadr_getAddress();
        p->header.dest = UB_ADDRESS_MASTER;
        uint8_t classes[] = UB_CLASSES;
        if( flags & 0x01 ){
            flags ^= 0x01;
            p->header.flags = UB_PACKET_MGT | UB_PACKET_NOACK | UB_PACKET_UNSOLICITED;
            p->data[0] = MGT_BRIDGEDISCOVER;
            p->data[1] = classes[0];
            p->data[2] = classes[1];
            p->data[3] = classes[2];
            p->data[4] = classes[3];
            strcpy((char*)p->data+5,(char*)ubadr_getID());
            p->header.len = strlen((char*)p->data+5)+5;
        }else if( flags & 0x02 ){
            flags ^= 0x02;
            p->header.flags = UB_PACKET_MGT | UB_PACKET_UNSOLICITED;
            p->data[0] = MGT_BRIDGEALIVE;
            p->header.len = 1;
        }
        ubpacket_send();
    }
}

