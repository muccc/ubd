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
#include "ubpacket.h"
#include "ubaddress.h"
#include "ub.h"

volatile uint8_t gotms = 0;
ISR(TIMER0_COMPA_vect)
{
    gotms = 1;
}

int main(void)
{
    
    //uint16_t i = 0;
    DDRA = 0xFF;
    
    TCCR0A = (1<<WGM01);
    TCCR0B = (1<<CS02) | (1<<CS00);
    OCR0A=17;       //~1ms  
    TIMSK0|=(1<<OCIE0A);

    //uart1_init( UART_BAUD_SELECT(115200,F_CPU));
    //uart1_puts("reset\n");
    ub_init(UB_SLAVE);
    //ubadr_setAddress(0x11);
    //rs485slave_setConfigured(1);
    sei();

    struct ubpacket_t * out = ubpacket_getSendBuffer();
    out->header.src = ubadr_getAddress();
    out->header.dest = 1;
    out->header.len = 1;
    out->data[0] = 'c';
    
    //ubpacket_send();

    //uint8_t new = 1;
    while(1){
        /*if(  ubpacket_done() )
             ubpacket_send();*/
         ubpacket_done();
        //rs485slave_send((uint8_t *)c, strlen(c));
        ub_process();
        if( ubpacket_gotPacket() ){
            /*struct ubpacket_t * in = ubpacket_getIncomming();
            if( in->header.src == 1 ){
                struct ubpacket_t * out = ubpacket_getSendBuffer();
                out->header.src = in->header.dest;
                out->header.dest = 1;
                out->header.len = 0;
                ubpacket_send();
            }*/
            ubpacket_processed();
            PORTA ^= 0x02;
        }

        if( gotms ){
            gotms = 0;
            ub_tick();
        }
    }
}
