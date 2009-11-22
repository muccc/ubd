#include <string.h>
#include <avr/io.h>

#include "ub.h"
#include "ubpacket.h"
#include "ubslavemgt.h"
#include "ubaddress.h"

#include "settings.h"

enum mgtstate {
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
    uint8_t * d = p->data;
    PORTB ^= (1<<PB0);
    if(!(p->header.flags & UB_PACKET_MGT))
        return 0;

    switch(d[0]){
        case 'S':
            //d[p->len] = 0;                  //TODO: check bufferlen
            //if( d[2] == '"' )
            if(ubadr_compareID(d+2)){

                ubadr_setAddress(d[1]);
                ubconfig.configured = 1;
                //switch(ubslavemgt_state){
                //    case DISCOVER:
                        ubslavemgt_state = IDENTIFY;
                //    break;
                //}
            }
        break;
        case 'O':
            //switch(ubslavemgt_state){
            //    case IDENTIFY:
                    ubslavemgt_state = CONNECTED;
            //    break;
            //}
        break;
        case 's':
            settings_setid(d+1);
        break;
        case 'r':
            while(1);
        break;
        /*case 'g':
            p = packet_getSendBuffer();
            p->dest = UB_ADDRESS_BROADCAST;
            p->flags = 0;
            p->data[0] = 'M';
            p->data[1] = 'N';
            settings_readid(p->data+2);
            p->len = strlen((char*)p->data);
            packet_send();
       break;*/
    }
    return 1;
}

void ubslavemgt_tick(void)
{
    struct ubpacket_t * p;
    static uint16_t time = 0;
    static uint16_t blubb = 0;
    if(!time--){
        time = 1000;
    }

    if(time == 0){
        p = ubpacket_getSendBuffer();
        switch(ubslavemgt_state){
            case DISCOVER:
                p->header.src = ubadr_getAddress();
                p->header.dest = UB_ADDRESS_BROADCAST;
                p->header.flags = UB_PACKET_MGT;
                if( ubconfig.master ){
                    p->data[0] = MGT_MASTER;
                }else if( ubconfig.slave ){
                    p->data[0] = MGT_DISCOVER;
                }
                strcpy((char*)p->data+1,(char*)ubadr_getID());
                p->header.len = strlen((char*)p->data);
                ubpacket_send();
            break;
            case IDENTIFY:
                p->header.dest = UB_ADDRESS_MASTER;
                p->header.src = ubadr_getAddress();
                p->header.flags = UB_PACKET_MGT;
                p->data[0] = MGT_IDENTIFY;
                strcpy((char*)p->data+1,(char*)ubadr_getID());
                p->header.len = strlen((char*)p->data);
                ubpacket_send();
            break;
            case CONNECTED:
                if( ubpacket_free() ){//  && blubb-- == 0){
                    blubb = 10;
                    p->header.dest = UB_ADDRESS_MASTER;
                    p->header.src = ubadr_getAddress();
                    p->header.flags = UB_PACKET_MGT;
                    p->data[0] = MGT_ALIVE;
                    p->header.len = 1;
                    ubpacket_send();
                }
           break;
        }
    }
}
