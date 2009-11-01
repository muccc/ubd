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

    //ubstat_addNode(45,0xFF);
    //ubstat_addNode(55,0xFF);
    //ubstat_addNode(95,0xFF);
    
    if( rs485master_setQueryInterval(0x10, 100) == UB_ERROR )
        while(1);
    rs485master_setQueryInterval(0x11, 100);
    sei();
    struct ubpacket_t out;
    out.header.dest = 0x11;
    out.header.src = 1;
    out.header.flags |= UB_PACKET_SEQ;
    out.header.len = 5;
    strcpy((char*)out.data, "test0");
    uint8_t lastrx = 0x11;
    while(1){
        ub_process();
        struct ubpacket_t p;
        int16_t len =  0; //rs485master_getPacket(&p);
        if( ubpacket_gotPacket() ){
            struct ubpacket_t * in = ubpacket_getIncomming();
            if( p.header.src == 0x11 )
                lastrx = p.header.src;
            //serial_putStart();
            //serial_putenc((uint8_t *)in,in->header.len + sizeof(in->header));
            //serial_putStop();

            ubpacket_processed();
        }
        if( gotms ){
            gotms = 0;
            if(i++==150){
                if( lastrx == 0x11 ){
                    i = 0;
                    if( 0 && rs485master_sendPacket(&out) == UB_OK ){
                        out.data[4]++;
                        out.header.flags ^= UB_PACKET_SEQ;
                        lastrx = 0;
                    }else{
                        i--;
                    }
                }else{
                    i--;
                }

            }
            ub_tick();
        }
        
    };
}
