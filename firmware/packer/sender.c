#include <stdint.h>
#include "timer.h"
#include <string.h>

#include "hal.h"
#include "bus.h"
#include "busuart.h"
#include "random.h"
#include "sender.h"
#include "frame.h"
#include "packet.h"
#include "busmgt.h"

volatile uint8_t tick;
volatile uint16_t time;
void sender_init(uint8_t addr)
{
    packet_init(0,0);
    busmgt_init();

    tick = 0;
    time = 0;
    DDRB |= (1<<PB0);
}

void sender_mainloop(void)
{
    uint8_t first = PIND & (1<<PD7);
    first = 0;
    struct ubpacket * p;
    uint8_t data = 0;
    while (1){
        if(tick){
            tick = 0;
            time++;
            packet_tick();
            busmgt_tick();
            //if(time == 1000){
            //if( packet_done() ){
            if(packet_gotPacket() || first){
                //PORTB |= (1<<PB0);
                first = 0;
                packet_processed();
                p = packet_getSendBuffer();
                if( PIND & (1<<PD7) )
                    p->dest = 'B';
                else
                    p->dest = 'A';
                p->dest = 1;
                p->flags = 0;
                p->len = 1;
                p->data[0] = data++;
                packet_send();
                //PORTB &= ~(1<<PB0);
            }
            //if(packet_done() && time > 1000)
            //    time = 0;
        }
        wdt_reset();
    }
}

void sender_tick(void)
{
    tick=1;
}
