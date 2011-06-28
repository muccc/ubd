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
#include "digitaloutput.h"
#include "digitalinput.h"

int main(void) {
    int i = 0;
    wdt_enable(WDTO_2S);

    DDRC |= (1<<PC5);   // LED

#ifdef UB_ENABLERF 
    DDRB |= (1<<PB4);   // SS has to be an output or SPI will switch to slave mode
    PORTB &= ~(1<<PB4);
    ub_init(UB_SLAVE, UB_RF, 0);
#endif
#ifdef UB_ENABLERS485
    ub_init(UB_SLAVE, UB_RS485, 0);
#endif

    timer0_init();
    digitalinput_init();
    digitaloutput_init();
    sei();

    while (1) {
        wdt_reset();
        ub_process();
        digitalinput_process();
        if( ubpacket_gotPacket() ){
            struct ubpacket_t * out = ubpacket_getSendBuffer();
            struct ubpacket_t * in = ubpacket_getIncomming();
            if( in->header.class == UB_CLASS_DIGITALOUTPUT ){
                digitaloutput_cmd(in,out);
                out->header.class = UB_CLASS_DIGITALOUTPUT;
            }else if( in->header.class == UB_CLASS_DIGITALINPUT ){
                digitalinput_cmd(in,out);
                out->header.class = UB_CLASS_DIGITALINPUT;
            }
            ubpacket_processed();
        }
        
        if( timebase ){
            timebase = 0;
            ub_tick();
            if( i++ == 1000 ){
                i = 0;
            //    PORTC ^= (1<<PC5);
            }
        }
    }
}
