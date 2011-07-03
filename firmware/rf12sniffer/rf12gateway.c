#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include "config.h"
#include "uart.h"
#include "rf12.h"
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
    rf12_init();
    rf12_setfreq(RF12FREQ(434.32));
    rf12_setbandwidth(4, 1, 4);     // 200kHz Bandbreite,
                                    //-6dB Verstärkung, DRSSI threshold: -79dBm
    rf12_setbaud(19200);
    rf12_setpower(0, 6);            // 1mW Ausgangangsleistung,
                                    // 120kHz Frequenzshift
    //PORTA = 0;
    DDRA = 0xFF;
    sei();
    serial_sendFrames("Hello wordl");
    rf12_rxstart();
    uint16_t outlen = 0;
    uint8_t out = 0;

    while(1){
        uint16_t len = rf12_rxfinish(indata);
        switch( len ){
            case 255:
            case 254:
            break;
            default:
            serial_putStart();
            serial_putcenc('P');
            serial_putenc(indata,len);
            serial_putStop();
            rf12_rxstart();
            break;
        }
        if( !out && outlen == 0 )
            outlen = serial_readline(outdata, 512);
        if( !out && outlen ){
            //serial_sendFrames("S");
            while( (rf12_trans(0x0000)>>8) & (1<<0) );
            rf12_allstop();
            rf12_txstart(outdata, outlen);
            out = 1;
        }
        if( out && (rf12_txfinished() == 0) ){
            serial_sendFrames("S");
            out = 0;
            outlen = 0;
            rf12_rxstart();
        }
    }
}

