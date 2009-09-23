//#include "ubconfig.h"
#include "ub.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <string.h>
#include "ubstat.h"
#include "ubrs485slave.h"
#include "usart.h"
#include "serial_handler.h"

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

    ub_init(UB_SLAVE);
    ub_setAddress(0x10);
    sei();
    char c[] = "newslaveID";
    while(1){
        rs485client_send((uint8_t *)c, strlen(c));
        ub_process();
        if( gotms ){
            gotms = 0;
            if(i++==500){
                i = 0;
            }
            ub_tick();
        }
    }
}
