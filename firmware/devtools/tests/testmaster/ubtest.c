//#include "ubconfig.h"
#include "ub.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <string.h>

#include "ubstat.h"
#include "ubrs485master.h"
#include "usart.h"
#include "serial_handler.h"
#include "ubpacket.h"

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
    OCR0A=17;       //~1ms  
    TIMSK0|=(1<<OCIE0A);

    uart1_init( UART_BAUD_SELECT(115200,F_CPU));
    uart1_puts("\\1hello world\\2");
    ub_init(UB_MASTER);

    sei();
    while(1){
        ub_process();
        if( ubpacket_gotPacket() ){
            ubpacket_processed();
        }
        if( gotms ){
            gotms = 0;
            ub_tick();
        }   
    }
}
