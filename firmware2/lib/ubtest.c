//#include "ubconfig.h"
#include "ub.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include "ubstat.h"

volatile uint8_t gotms = 0;
ISR(TIMER0_COMPA_vect)
{
    gotms = 1;
}

int main(void)
{
    uint16_t i = 0;
    DDRA = 0xFF;
    TCCR0A = (1<<WGM01);
    TCCR0B = (1<<CS02) | (1<<CS00);
    OCR0A=17;

    TIMSK0|=(1<<OCIE0A);
    ub_init(UB_MASTER);

    ubstat_addNode(45,0xFF);
    ubstat_addNode(55,0xFF);
    ubstat_addNode(95,0xFF);
    sei();
    while(1){
        ub_process();
        if( gotms ){
            gotms = 0;
            if(i++==1024){
                i = 0;
                //PORTA ^= 0x01;
            }
            ub_tick();
        }
    };
}
