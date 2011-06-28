#include <avr/wdt.h>
#include <string.h>
#include <stdio.h>

#include "ub.h"
#include "ubpacket.h"
#include "ubslavemgt.h"
#include "ubaddress.h"


enum slavemgtstate {
    DISCOVER,
    IDENTIFY,
    CONNECTED
};

uint8_t ubslavemgt_state;

void ubslavemgt_init(void)
{
    ubslavemgt_state = DISCOVER;
}

uint8_t ubslavemgt_process(struct ubpacket_t * p)
{
    uint8_t * data = p->data;
    struct ubpacket_t * out;

    if(!(p->header.flags & UB_PACKET_MGT))
        return 0;
    if(!(p->header.src == UB_ADDRESS_MASTER))
        return 1;

    switch(data[0]){
        case 'S':
            //d[p->len] = 0;                  //TODO: check bufferlen
            if(ubadr_compareID(data+2)){
                ubadr_setAddress(data[1]);
                ubconfig.configured = 1;
                ubslavemgt_state = IDENTIFY;
#ifdef UB_ENABLERS485
                if( ubconfig.rs485slave )
                    rs485slave_setConfigured(1);
#endif
            }
        break;
        case 'O':
            ubslavemgt_state = CONNECTED;
        break;
        case 's':
            ubadr_setID(data+1);
        break;
        case 'r':
            wdt_enable(WDTO_30MS);
            while(1);
        break;
        case 'V':
            out = ubpacket_getSendBuffer();
            out->header.dest = UB_ADDRESS_MASTER;
            out->header.src = ubadr_getAddress();
            out->header.flags |= UB_PACKET_MGT;
            sprintf((char *)out->data,"V="__DATE__);
            out->header.len = strlen((char*)out->data);
        break;
        case 'A':
            ubadr_addMulticast(data[1]);
        break;
        case 'R':
            ubadr_removeMulticast(data[1]);
        break;
    }
    return 2;
}

void ubslavemgt_tick(void)
{
    struct ubpacket_t * p;
    static uint16_t time = 0;
    if(!time--){
        time = 10000;
    }
    //return;
    if(time == 0){
        p = ubpacket_getSendBuffer();
        p->header.src = ubadr_getAddress();
        p->header.flags = UB_PACKET_MGT | UB_PACKET_UNSOLICITED;

        switch(ubslavemgt_state){
            case DISCOVER:
                p->header.dest = UB_ADDRESS_BROADCAST;
                p->data[0] = MGT_DISCOVER;
                p->data[1] = UB_INTERVAL>>8;
                p->data[2] = UB_INTERVAL&0xFF;
                uint8_t classes[] = UB_CLASSES;
                p->data[3] = classes[0];
                p->data[4] = classes[1];
                p->data[5] = classes[2];
                p->data[6] = classes[3];

                strcpy((char*)p->data+7,(char*)ubadr_getID());
                p->header.len = strlen((char*)p->data+7)+7;
                ubpacket_send();
            break;
            case IDENTIFY:
                p->header.dest = UB_ADDRESS_MASTER;
                p->data[0] = MGT_IDENTIFY;
                strcpy((char*)p->data+1,(char*)ubadr_getID());
                p->header.len = strlen((char*)p->data);
                ubpacket_send();
            break;
            case CONNECTED:
                if( ubpacket_free() ){
                    p->header.dest = UB_ADDRESS_MASTER;
                    p->data[0] = MGT_ALIVE;
                    p->header.len = 1;
                    ubpacket_send();
                }
                time = 15000;
           break;
        }
    }
}
