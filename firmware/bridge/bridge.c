#include <stdint.h>
#include "timer.h"
#include "hal.h"
#include "bus.h"
#include "uart.h"
#include "random.h"
#include "bridge.h"
#include "frame.h"

void bridge_init(uint8_t addr)
{
}

void bridge_mainloop(void)
{
	while (1){
        if( bus_frame->isnew == 1){
            bus_frame->isnew = 0;
        }
        wdt_reset();
	}
}

void bridge_tick(void)
{
}
