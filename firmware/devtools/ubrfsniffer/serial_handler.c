#include <avr/io.h>
#include <stdint.h>
#include <string.h>
#include "uart.h"
#include "serial_handler.h"

uint8_t buffer[SERIAL_BUFFERLEN];
//uint8_t serial_buffer[SERIAL_BUFFERLEN];

inline void serial_putcenc(uint8_t c)
{
    if(c == SERIAL_ESCAPE)
        uart1_putc(SERIAL_ESCAPE);
    uart1_putc(c);
}

void serial_putsenc(char * s)
{
    while(*s){
        if(*s == SERIAL_ESCAPE)
            uart1_putc(SERIAL_ESCAPE);
        uart1_putc(*s++);
    }
}

void serial_putenc(uint8_t * d, uint16_t n)
{
    uint16_t i;
    for(i=0;i<n;i++){
        if(*d == SERIAL_ESCAPE)
            uart1_putc(SERIAL_ESCAPE);
        uart1_putc(*d++);
    }
}

inline void serial_putStart(void)
{
    uart1_putc(SERIAL_ESCAPE);
    uart1_putc(SERIAL_START);
}

inline void serial_putStop(void)
{
    uart1_putc(SERIAL_ESCAPE);
    uart1_putc(SERIAL_END);
}

void serial_sendFrames(char * s)
{
    serial_putStart();
    serial_putsenc(s);
    serial_putStop();
}

void serial_sendFramec(uint8_t s)
{
    serial_putStart();
    serial_putcenc(s);
    serial_putStop();
}

uint16_t serial_readline(uint8_t * buf, uint16_t max)
{
    uint16_t l;

    l = readline();
    if( l ){
        if( l > max )
            l = max;
        memcpy(buf,buffer,l);
    }
    return l;
}

uint16_t readline( void )
{
    static int fill = 0;
    static uint8_t escaped = 0;
    int  i = uart1_getc();
    uint8_t data;
    
    if ( i & UART_NO_DATA ){
        return 0;
    }
    data = i&0xFF;
/*serial_putStart();
serial_putcenc('D');
serial_putcenc(data);
serial_putStop();*/
    if(data == SERIAL_ESCAPE){
        if(!escaped){
            escaped = 1;
            return 0;
        }
        escaped = 0;
    }else if(escaped){
        escaped = 0;
        if(data == SERIAL_START){
            fill = 0;
            return 0;
        }else if( data == SERIAL_END){
            return fill;
        }
    }
    buffer[fill++] = data;
    if(fill >= SERIAL_BUFFERLEN)
        fill = SERIAL_BUFFERLEN -1;
    return 0;
}

