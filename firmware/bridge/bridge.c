#include <stdint.h>
#include "timer.h"
#include "hal.h"
#include "bus.h"
#include "uart.h"
#include "random.h"
#include "bridge.h"
#include "frame.h"
#include "serial_handler.h"
//#define DEBUG(...) {printf(__VA_ARGS__);}

void bridge_init(uint8_t addr)
{
}

void bridge_mainloop(void)
{
   struct frame * f;
	while (1){
        f = bus_frame;
        if( f->isnew == 1){
            DEBUG("new frame len=%u",f->len);
            f->isnew = 0;
        }
        wdt_reset();
	}
}

void bridge_tick(void)
{
}
