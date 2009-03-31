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

volatile uint8_t tick;
volatile uint16_t time;
void sender_init(uint8_t addr)
{
    packet_init();
    tick = 0;
    time = 0;
}

void sender_mainloop(void)
{
    struct ubpacket * p;
    while (1){
        if(tick){
            tick = 0;
            time++;
            packet_tick();
            if(time == 1000){
                p = packet_getSendBuffer();
                p->dest = 0x42;
                p->flags = UB_PACKET_UNICAST;
                p->len = 1;
                p->data[0] = 'A';
                packet_send();
            }
            if(packet_done() && time > 1000)
                time = 0;
        }
        wdt_reset();
    }
}

void sender_tick(void)
{
    tick=1;
}
