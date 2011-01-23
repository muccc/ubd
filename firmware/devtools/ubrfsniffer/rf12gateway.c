#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include "config.h"
#include "uart.h"
#include "ubrf12.h"
#include "serial_handler.h"
#include "leds.h"

uint8_t indata[256];
uint8_t outdata[512];

int main(void)
{
    wdt_disable();
    /* Clear WDRF in MCUSR */
    MCUSR &= ~(1<<WDRF);
    /* Write logical one to WDCE and WDE */
    /* Keep old prescaler setting to prevent unintentional time-out */
    WDTCSR |= (1<<WDCE) | (1<<WDE);
    /* Turn off WDT */
    WDTCSR = 0x00;
    leds_init();
    uart1_init( UART_BAUD_SELECT(UART_BAUDRATE,F_CPU));
    volatile unsigned long l;
    for(l=0;l<10000;l++);
    ubrf12_init();
    ubrf12_setfreq(RF12FREQ(434.32));
    ubrf12_setbandwidth(4, 1, 4);     // 200kHz Bandbreite,
                                    //-6dB Verstärkung, DRSSI threshold: -79dBm
    ubrf12_setbaud(19200);
    ubrf12_setpower(0, 6);            // 1mW Ausgangangsleistung,
                                    // 120kHz Frequenzshift
    //PORTA = 0;
    DDRA = 0xFF;
    sei();
    serial_sendFrames("Hello world");
    while(1){
        ubrf12_rxstart();
        uint16_t len = ubrf12_rxfinish(indata);
        switch( len ){
            case 255:
            case 254:
            break;
            default:
            serial_putStart();
            serial_putcenc('P');
            serial_putenc(indata,len);
            serial_putStop();
            ubrf12_rxstart();
            break;
        }
    }
}

