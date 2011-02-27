#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <util/delay.h>

#include <string.h>

#include "uart.h"
#include "leds.h"
#include "pinutils.h"
#include "config.h"

#define RS485_BITRATE   115200

int main(void)
{
    uint8_t escaped = 0;
    leds_init();

    DDR_CONFIG_OUT(RS485_TX_PIN);
    DDR_CONFIG_OUT(RS485_NRX_PIN);

    PIN_CLEAR(RS485_TX_PIN);
    PIN_CLEAR(RS485_NRX_PIN);

    UBRR0 = UART_BAUD_SELECT(RS485_BITRATE,F_CPU);
    UCSR0B |= (1<<TXEN0) | (1<<RXEN0);

    UBRR1 = UART_BAUD_SELECT(RS485_BITRATE,F_CPU);
    UCSR1B |= (1<<TXEN1) | (1<<RXEN1);
    
    wdt_reset();
    PIN_SET(RS485_TX_PIN);
    PIN_SET(RS485_NRX_PIN);

    while(1){
        wdt_reset();
        if( (UCSR1A & (1<<RXC1)) ){
            uint8_t data = UDR1;
            if( escaped ){
                escaped = 0;
                if( data == 'R' ){
                    wdt_enable(WDTO_2S);
                    while(1);
                }
            }else{
               if( data == '\\' ){
                    escaped = 1;
                    continue;
                }
            }
            UDR0 = data;
            PIN_SET(RS485_TX_PIN);
            PIN_SET(RS485_NRX_PIN);
            //_delay_us(4);
            leds_tx();
            while( !(UCSR0A & (1<<TXC0)) );
            UCSR0A |= (1<<TXC0);
            leds_txend();
            //_delay_us(4);
            PIN_CLEAR(RS485_TX_PIN);
            PIN_CLEAR(RS485_NRX_PIN);
        }
        if( (UCSR0A & (1<<RXC0)) ){
            UDR1 = UDR0;
            leds_rx();
            while( !(UCSR1A & (1<<TXC1)) );
            UCSR1A |= (1<<TXC1);
            leds_rxend();
        }

    }
}
