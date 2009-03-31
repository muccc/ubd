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

void sender_init(uint8_t addr)
{
    packet_init();
    tick = 0;
}

void sender_mainloop(void)
{
    while (1){
        if(tick){
            tick = 0;
            packet_tick();
        }
        wdt_reset();
    }
}

void sender_tick(void)
{
    tick=1;
}
