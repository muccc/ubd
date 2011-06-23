#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include "usart.h"
#include "ubrf12.h"
#include "serial_handler.h"
#include "ubleds.h"

uint8_t indata[128];
uint8_t outdata[128];

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
    ubleds_init();
    uart1_init( UART_BAUD_SELECT(115200,F_CPU));
    volatile unsigned long l;
    for(l=0;l<10000;l++);
    ubrf12_init(1);
    ubrf12_setfreq(RF12FREQ(434.32));
    ubrf12_setbandwidth(4, 1, 4);     // 200kHz Bandbreite,
                                    //-6dB Verstärkung, DRSSI threshold: -79dBm
    ubrf12_setbaud(19200);
    ubrf12_setpower(0, 6);            // 1mW Ausgangangsleistung,
                                    // 120kHz Frequenzshift
    //PORTA = 0;
    DDRA = 0xFF;
    sei();
    //serial_sendFrames("Hello wordl");
    ubrf12_rxstart();
    uint16_t outlen = 0;
    uint8_t out = 0;
    uint16_t crc;
    while(1){
        ubrf12_rxstart();
        uint16_t len = ubrf12_rxfinish(indata);
        switch( len ){
            case 255:
            case 254:
            break;
            default:
                crc = ubcrc16_data(indata, len-2);
                if( (crc>>8) == indata[len-2] &&
                    (crc&0xFF) == indata[len-1] ){
                    serial_putStart();
                    serial_putcenc('P');
                    serial_putenc(indata,len-2);
                    serial_putStop();
                }else{
                    serial_sendFrames("DCRC");
                }
                ubrf12_rxstart();
            break;
        }
        if( !out && outlen == 0 )
            outlen = serial_readline(outdata, 126);
        if( !out && outlen ){
            //serial_sendFrames("S");
            crc = ubcrc16_data(outdata, outlen);
            outdata[outlen] = crc >> 8;
            outdata[outlen+1] = crc & 0xFF;

            while( (ubrf12_trans(0x0000)>>8) & (1<<0) );
            ubrf12_allstop();
            ubrf12_txstart(outdata, outlen+2);
            out = 1;
        }
        if( out && (ubrf12_txfinished() == 0) ){
            serial_sendFrames("S");
            out = 0;
            outlen = 0;
        }
    }
}

