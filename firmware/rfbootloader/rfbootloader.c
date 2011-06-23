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
    int i = 0;
    //LED
    DDRC |= (1<<PC5);
    //High power output
    DDRD |= (1<<PD4);
    //Inputs
    PORTC |= (1<<PC0) | (1<<PC1) | (1<<PC6);
    //DDRD |= (1<<PD4) | (1<<PD5) | (1<<PD6) | (1<<PD7);
    //DDRB |= (1<<PB3) | (1<<PB4) | (1<<PB5) | (1<<PB6);
    
    wdt_enable(WDTO_2S);
    //uart1_init( UART_BAUD_SELECT(115200,F_CPU));

    DDRB |= (1<<PB4);   //SS has to be an output or SPI will switch to slave mode
    PORTB &= ~(1<<PB4);
    ub_init(UB_SLAVE, UB_RF, 0);
    
    sei();
    timer0_init();
    while (1) {
        wdt_reset();
        ub_process();
        if( ubpacket_gotPacket() ){
            struct ubpacket_t * out = ubpacket_getSendBuffer();
            struct ubpacket_t * in = ubpacket_getIncomming();
            if( in->header.class == UB_CLASS_BOOTLOADER ){
                PORTD ^= (1<<PD4);
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
