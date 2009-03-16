#include <stdint.h>
#include "timer.h"
#include "hal.h"
#include "bus.h"
#include "busuart.h"
#include "random.h"
#include "bridge.h"
#include "frame.h"
#include "serial_handler.h"
#include <string.h>

void bridge_init(uint8_t addr)
{
}
volatile uint8_t flag;
void bridge_mainloop(void)
{
    struct frame * f;
    struct frame s;
    s.len = 6;
    strcpy((char*)s.data,"FNORD2");
	while (1){
        cli();
        f = bus_frame;
        sei();
        if( f->isnew == 1){
            f->data[f->len]=0;
            //DEBUG("new frame len=%u data=%s",f->len, f->data);
            serial_putStart();
            serial_putcenc('I');
            serial_putenc((char*)f,f->len);
            serial_putcenc(f->crc & 0xFF);
            serial_putcenc((f->crc >> 8)  & 0xFF);
            serial_putStop();
            f->isnew = 0;
        }
        uint8_t len = serial_readline();
        if(len){
            s.len = len;
            memcpy(s.data,serial_buffer,len);
            bus_send(&s,1);
        }
        uint8_t r = bus_done();
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
    static uint16_t i = 1;
    if(--i == 0){
        flag = 1;
        i = 500;
    }
}
