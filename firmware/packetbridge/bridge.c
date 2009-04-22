#include <stdint.h>
#include "timer.h"
#include "hal.h"
#include "random.h"
#include "bridge.h"
#include "frame.h"
#include "serial_handler.h"
#include "packet.h"

#include <string.h>
volatile uint8_t tick;

void bridge_init(uint8_t addr)
{
    tick = 0;
    packet_init(1);
}


void bridge_mainloop(void)
{
    uint8_t dest;
    uint8_t src;
    uint8_t data;

    struct ubpacket * p;
    while (1){
        if(tick){
            PORTC |= (1<<PC3);
            tick = 0;
            packet_tick();

            if( packet_gotPacket() ){
                DEBUG("g");
                p = packet_getIncomming();
                //DEBUG("new packet len=%u data=%s",p->len, p->data);
                serial_putStart();
                serial_putcenc('P');
                serial_putenc((unsigned char*)p,p->len+UB_PACKET_HEADER);
                serial_putStop();
                dest = p->dest;
                src = p->src;
                data = p->data[0];
                packet_processed();
                /*p = packet_getSendBuffer();
                p->dest= src;
                p->src = dest;
                p->flags = 0;
                p->len = 1;
                p->data[0] = data;
                packet_send();*/
            }
            PORTC &= ~(1<<PC3);
        }
        uint8_t len = serial_readline();
        if(len){
            //DEBUG("l");
            p = packet_getSendBuffer();
            p->dest= src;
            p->src = dest;
            p->flags = 0;
            p->len = 1;
            p->data[0] = data;
            memcpy(p,serial_buffer,len);
            packet_send();
            //DEBUG("s");
        }

        uint8_t r = packet_done();
        switch(r){
            case 0:
                break;
            case 1:
                serial_putStart();
                serial_putcenc('S');
                serial_putStop();
            break;
            default:
                serial_putStart();
                serial_putcenc('T');
                serial_putStop();
            break;
        }
        wdt_reset();
    }
}

void bridge_tick(void)
{
    tick = 1;
}
