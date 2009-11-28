//#include <avr/io.h>
#include <string.h>
#include <stdio.h>

#include "ub.h"
#include "ubpacket.h"
#include "ubmastermgt.h"
#include "ubaddress.h"

#include "settings.h"
#include "ubstat.h"
#include "ubrs485master.h"

#define  MGT_BRIDGEDISCOVER     'B'
#define  MGT_MASTERALIVE        'A'

enum slavemgtstate {
    DISCOVER,
    CONNECTED
};

uint8_t ubmastermgt_state;

void ubmastermgt_init(void)
{
    ubmastermgt_state = DISCOVER;
}

uint8_t ubmastermgt_process(struct ubpacket_t * p)
{
    uint8_t * d = p->data;
    struct ubstat_t * flags;
    struct ubpacket_t * out;
    if(!(p->header.flags & UB_PACKET_MGT))
        return 0;
    switch(d[0]){
        case 'q':
            flags = ubstat_getFlags(d[1]);
            rs485master_setQueryInterval(d[1], (d[2] << 8) + d[3]);
            flags->known = 1;
            //TODO: check if it'ss on the rf
            flags->rs485 = 1;
        break;
        case 'O':
            ubconfig.configured = 1;
            ubmastermgt_state = CONNECTED;
        break;
        case 'r':
            //TODO: get the reset to the other nodes too
            while(1);
        break;
        case 'V':
            out = ubpacket_getSendBuffer();
            out->header.dest = UB_ADDRESS_MASTER;
            out->header.src = ubadr_getAddress();
            out->header.flags = UB_PACKET_MGT;
            sprintf((char *)out->data,"D="__DATE__);
            out->header.len = strlen((char*)out->data);
            ubpacket_send();
        break;
    }
    return 1;
}

void ubmastermgt_tick(void)
{
    struct ubpacket_t * p;
    static uint16_t time = 0;
    if(!time--){
        time = 1000;
    }

    if(time == 0){
        p = ubpacket_getSendBuffer();
        switch(ubmastermgt_state){
            case DISCOVER:
                p->header.src = ubadr_getAddress();
                p->header.dest = UB_ADDRESS_MASTER;
                p->header.flags = UB_PACKET_MGT | UB_PACKET_NOACK;
                p->data[0] = MGT_BRIDGEDISCOVER;
                strcpy((char*)p->data+1,(char*)ubadr_getID());
                p->header.len = strlen((char*)p->data);
                ubpacket_send();
            break;
            case CONNECTED:
                if( ubpacket_free() ){
                    p->header.dest = UB_ADDRESS_MASTER;
                    p->header.src = ubadr_getAddress();
                    p->header.flags = UB_PACKET_MGT;
                    p->data[0] = MGT_MASTERALIVE;
                    p->header.len = 1;
                    ubpacket_send();
                }
            break;
        }
    }
}

