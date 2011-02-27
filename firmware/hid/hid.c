#include <avr/io.h>
#include <stdint.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <string.h>
#include <avr/wdt.h>

#include "ub.h"
#include "ubaddress.h"
#include "ubpacket.h"
#include "timer0.h"

int main(void) {
    ub_init(UB_SLAVE, UB_RF, 0);
    wdt_enable(WDTO_2S);
    timer0_init();
    sei();

    while (1) {
        wdt_reset();
        ub_process();
        if( ubpacket_gotPacket() ){
            struct ubpacket_t * out = ubpacket_getSendBuffer();
            struct ubpacket_t * in = ubpacket_getIncomming();
            if( in->header.len > 0 ) switch( in->data[0] ){
                case 'S':
                break;
                case 's':
                break;
            }
            out->header.class = UB_CLASS_HID;
            ubpacket_processed();
        }
        if( timebase ){
            timebase = 0;
            ub_tick();
        }
    }
}
