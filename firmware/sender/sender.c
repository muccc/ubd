#include <stdint.h>
#include "timer.h"
#include <string.h>

#include "hal.h"
#include "bus.h"
#include "busuart.h"
#include "random.h"
#include "sender.h"
#include "frame.h"

volatile uint8_t tick;
uint16_t send;

void sender_init(uint8_t addr)
{
    tick = send = 0;
}

void sender_mainloop(void)
{
    struct frame * f;
    struct frame s;
    s.len = 12;
    strcpy((char*)s.data,"FNORD23FUBAR");
    while (1){
        cli();
        f = bus_frame;
        sei();
        if( f->isnew == 1){
            f->isnew = 0;
        }
        if(send == 100){
            send = 0;
            bus_send(&s,1);
        }
        if(tick){
            tick = 0;
            send++;
            bus_tick();
        }
        wdt_reset();
    }
}

void sender_tick(void)
{
    //static uint16_t i = 1;
    //if(--i == 0){
    //    flag = 1;
    //    i = 1000;
    //}
    //bus_tick();
    tick=1;
}
