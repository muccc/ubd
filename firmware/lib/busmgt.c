#include <string.h>
#include "packet.h"
#include "busmgt.h"

enum mgtstate {
    DISCOVER,
    IDENTIFY,
    CONNECTED
};

uint8_t busmgt_state;
uint8_t busmgt_busmaster;

void busmgt_init(void)
{
    busmgt_state = DISCOVER;
}

void busmgt_tick(void)
{
    struct ubpacket * p;
    static uint16_t time = 0;
    if(!time--){
        time = 1000;
    }

/*    switch(busmgt_state){
        case DISCOVER:*/
            if(time == 0){
                p = packet_getSendBuffer();
                p->dest = UB_ADDRESS_BROADCAST;
                p->flags = 0;
                p->data[0] = MGT_DISCOVER;
                //settings_copyID(p->data+1);
                //strcpy((char*)p->data+1,"node1");
                //p->len = strlen((char*)p->data);
                p->len = 1;
                packet_send();
            }
/*        break;
        case IDENTIFY:
        break;
        case CONNECTED:
        break;
    }*/
}
