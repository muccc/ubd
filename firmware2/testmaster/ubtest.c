//#include "ubconfig.h"
#include "ub.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include "ubstat.h"
#include "ubrs485master.h"
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

    ub_init(UB_MASTER);

    ubstat_addNode(45,0xFF);
    ubstat_addNode(55,0xFF);
    ubstat_addNode(95,0xFF);
    
    if( rs485master_setQueryInterval(0x10, 100) == UB_ERROR )
        while(1);
    sei();
    uint8_t c[10];
    c[0] = 'h';
    c[1] = 'e';
    c[2] = 'l';
    c[3] = 'l';
    c[4] = '0';
    c[5] = 0;

    while(1){
        ub_process();
        uint8_t packet[30];
        uint8_t len = rs485master_getPacket(packet);
        if( len ){
            serial_putenc(packet,len);
        }
        if( gotms ){
            gotms = 0;
            if(i++==500){
                i = 0;
                if( rs485master_send(c, 6) == UB_OK )
                    c[5]++;
            }
            ub_tick();
        }
        
    };
}
