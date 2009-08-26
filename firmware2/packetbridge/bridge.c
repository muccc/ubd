#include <stdint.h>
#include <string.h>

#include "timer.h"
#include "hal.h"
#include "random.h"
#include "bridge.h"
#include "frame.h"
#include "serial_handler.h"
#include "packet.h"

volatile uint8_t tick;

void bridge_init(uint8_t addr)
{
    tick = 0;
    packet_init(1,0);
}

void bridge_mainloop(void)
{
    while (1){
        if(tick){
            PORTC |= (1<<PC3);
            tick = 0;
            packet_tick();
            PORTC &= ~(1<<PC3);
        }
        bridge_output();
        bridge_input();
        bridge_status();
        wdt_reset();
    }
}

void bridge_tick(void)
{
    tick = 1;
}

void bridge_status(void)
{
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

}

void bridge_input(void)
{
    struct ubpacket * p;
    uint8_t len = serial_readline();
    if(len) switch(serial_buffer[0]){
        case 'P':
            //DEBUG("S");
            p = packet_getSendBuffer();
            memcpy(p,serial_buffer+1,len-1);
            packet_send();
            //DEBUG("S");
            break;
        case 'I':
            packet_init(serial_buffer[1], serial_buffer[2]);
            break;
        case 'S':
            packet_setMode(serial_buffer[1]);
            break;
    } 
}

void bridge_output(void)
{
    struct ubpacket * p;
    if( packet_gotPacket() ){
        p = packet_getIncomming();
        //DEBUG("new packet len=%u data=%s",p->len, p->data);
        serial_putStart();
        serial_putcenc('P');
        serial_putenc((unsigned char*)p,p->len+UB_PACKET_HEADER);
        serial_putStop();
        packet_processed();
    }
}

