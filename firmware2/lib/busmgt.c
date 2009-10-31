#include <string.h>
#include <avr/io.h>

#include "ubpacket.h"
#include "busmgt.h"
#include "ubaddress.h"

enum mgtstate {
    DISCOVER,
    IDENTIFY,
    CONNECTED
};

uint8_t busmgt_state;

void busmgt_init(void)
{
    busmgt_state = DISCOVER;
}

uint8_t busmgt_process(struct ubpacket_t * p)
{
    uint8_t * d = p->data;
    PORTB ^= (1<<PB0);
    if(!(p->flags & UB_PACKET_MGT))
        return 0;

    switch(d[0]){
        case 'S':
            //d[p->len] = 0;                  //TODO: check bufferlen
            //if( d[2] == '"' )
            if(ubadr_compareID(d+3)){

                ubadr_setAddress(d[2]);
                ubconfig.configured = 1;
                //switch(busmgt_state){
                //    case DISCOVER:
                        busmgt_state = IDENTIFY;
                //    break;
                //}
            }
        break;
        case 'O':
            //switch(busmgt_state){
            //    case IDENTIFY:
                    busmgt_state = CONNECTED;
            //    break;
            //}
        break;
        /*case 's':
            settings_setID(d+2);
        break;
        case 'r':
            while(1);
        break;
        case 'g':
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

void busmgt_tick(void)
{
    struct ubpacket_t * p;
    static uint16_t time = 0;
    if(!time--){
        time = 1000;
    }

    if(time == 0){
        p = ubpacket_getSendBuffer();
        switch(busmgt_state){
            /*case DISCOVER:

                PORTA ^= 0x04;
                p->header.src = ubadr_getAddress();
                p->header.dest = UB_ADDRESS_BROADCAST;
                p->header.flags = 0;
                p->data[0] = 'M';
                p->data[1] = MGT_DISCOVER;
                strcpy((char*)p->data+2,(char*)ubadr_getID());
                //strcpy((char*)p->data+2,"node1");
                p->header.len = strlen((char*)p->data);
                ubpacket_send();
            break;*/
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
                if( ubpacket_free() ){
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
