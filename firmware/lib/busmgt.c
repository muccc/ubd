#include <string.h>
#include <avr/io.h>

#include "packet.h"
#include "busmgt.h"
#include "settings.h"

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

uint8_t busmgt_process(struct ubpacket * p)
{
    uint8_t * d = p->data;
    PORTB ^= (1<<PB0);
    if(d[0] != 'M')
        return 0;

    switch(d[1]){
        case 'S':
            //d[p->len] = 0;                  //TODO: check bufferleb
            if(settings_compareid(d+4)){
                packet_setAdr(d[2]);
                packet_setMaster(d[3]);
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
        case 's':
            settings_setid(d+2);
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
       break;
    }
    return 1;
}

void busmgt_tick(void)
{
    struct ubpacket * p;
    static uint16_t time = 0;
    if(!time--){
        time = 1000;
    }

    if(time == 0){
        p = packet_getSendBuffer();
        switch(busmgt_state){
            case DISCOVER:
                p->dest = UB_ADDRESS_BROADCAST;
                p->flags = 0;
                p->data[0] = 'M';
                p->data[1] = MGT_DISCOVER;
                settings_readid(p->data+2);
                //strcpy((char*)p->data+2,"node1");
                p->len = strlen((char*)p->data);
                packet_send();
            break;
            case IDENTIFY:
                p->dest = packet_getMaster();
                p->flags = 0;
                p->data[0] = 'M';
                p->data[1] = MGT_IDENTIFY;
                settings_readid(p->data+2);
                //strcpy((char*)p->data+2,"node1");
                p->len = strlen((char*)p->data);
                packet_send();
            break;
            case CONNECTED:
                p->dest = packet_getMaster();
                p->flags = 0;
                p->data[0] = 'M';
                p->data[1] = MGT_ALIVE;
                //settings_copyID(p->data+1);
                //strcpy((char*)p->data+2,"node1");
                //p->len = strlen((char*)p->data);
                p->len = 2;
                packet_send();
           break;
        }
    }
}
