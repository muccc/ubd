#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <string.h>

#include "ubrf12.h"
#include "ubleds.h"
#include "ubcrc.h"
#include "flash.h"

#define CHUNKSIZE       32

uint8_t pagedata[SPM_PAGESIZE];
uint8_t data[100];
uint8_t id[100];
char * name = (char *)(E2END - 50);
volatile unsigned long timeout;
uint8_t selected = 0;

static void cmd(uint8_t cmd, uint8_t * data, uint16_t len);
static void boot(void);

void (*jump_to_application)(void) = (void *)0x0000;

static void boot(void)
{
    uint8_t tmp = MCUCR;
    MCUCR = tmp | (1<<IVCE);
    MCUCR = tmp & (~(1<<IVSEL));
    jump_to_application();
}

int main(void)
{
#if 0
    wdt_disable();
    /* Clear WDRF in MCUSR */
    MCUSR &= ~(1<<WDRF);
    /* Write logical one to WDCE and WDE */
    /* Keep old prescaler setting to prevent unintentional time-out */
    WDTCSR |= (1<<WDCE) | (1<<WDE);
    /* Turn off WDT */
    WDTCSR = 0x00;
    ubleds_init();
    volatile unsigned long l;
    while(1){
        if( timeout++ > 1000000UL ){
            timeout = 0;
            PORTC ^= (1<<PC5);
        }
    }
#else
    //wdt_enable(WDTO_500MS);

    uint8_t len;
    uint16_t crc;

    wdt_enable(WDTO_2S);
    uint8_t tmp = MCUCR;
    MCUCR = tmp | (1<<IVCE);
    MCUCR = tmp | (1<<IVSEL);
    ubleds_init();
    DDRB |= (1<<PB4);

    volatile unsigned long l;
    for(l=0;l<10000;l++)
        wdt_reset();

    ubrf12_init(1);
    wdt_reset();
    ubrf12_setfreq(RF12FREQ(434.32));
    ubrf12_setbandwidth(4, 1, 4);     // 200kHz Bandbreite,
                                    //-6dB Verstärkung, DRSSI threshold: -79dBm
    ubrf12_setbaud(19200);
    ubrf12_setpower(0, 6);            // 1mW Ausgangangsleistung,
                                    // 120kHz Frequenzshift
    sei();

    wdt_reset();
    eeprom_read_block(id,name,50);
    eeprom_read_block(data+1,name,50);

    ubrf12_allstop();
    data[0] = 'B';
    len = strlen((char*)data);

    crc = ubcrc16_data(data, len);
    data[len] = crc >> 8;
    data[len+1] = crc & 0xFF;
    ubrf12_txstart(data, len+2);
    
    timeout = 0;
    while(1){
        wdt_reset();
        ubrf12_rxstart();
        len = ubrf12_rxfinish(data);
        if( len != 0 && len != 255 && len !=254 ){
            crc = ubcrc16_data(data, len-2);
            if( (crc>>8) == data[len-2] &&
                    (crc&0xFF) == data[len-1] ){
                cmd(data[0],data+1,len-3);
            }
        }
        if( !selected && timeout++ > 1000000UL ){
        //if( !selected && timeout++ > 100000UL ){
            /*
            timeout = 0;
            ubrf12_allstop();
            data[0] = 'B';
            eeprom_read_block(data+1,name,50);
            len = strlen((char*)data);

            crc = ubcrc16_data(data, len);
            data[len] = crc >> 8;
            data[len+1] = crc & 0xFF;
            ubrf12_txstart(data, len+2);
            */
            boot();
        }
    }

#endif
}

static void cmd(uint8_t cmd, uint8_t * data, uint16_t len)
{
    uint8_t ret = 'f';
    uint8_t retlen = 1;

    if( cmd == 'S' ){
        data[len] = 0;
        if( strcmp((char*)id,(char*)data) == 0 ){
            selected = 1;
            ret = 'S';
        }else{
            selected = 0;
        }
    }

    if( !selected )
        return;

    switch(cmd){
        case 'D':
            if( len == CHUNKSIZE+1 ){
                memcpy(pagedata+data[0]*CHUNKSIZE, data+1, CHUNKSIZE);
                ret = cmd;
            }
        break;
        case 'F':
            flash_page(data[0] * SPM_PAGESIZE,pagedata);
            ret = cmd;
        break;
        case 'R':
            eeprom_read_block(data+1,(uint16_t*)(data[0]*CHUNKSIZE),CHUNKSIZE);
            ret = cmd;
            retlen = CHUNKSIZE+1;
        break;
        case 'E':
            eeprom_write_block(data+1,(uint16_t*)(data[0]*CHUNKSIZE),CHUNKSIZE);
            ret = cmd;
        break;
        case 'G':
            boot();
        break;
    }
    data[0] = ret;
    uint16_t crc = ubcrc16_data(data, retlen);
    data[retlen] = crc >> 8;
    data[retlen+1] = crc & 0xFF;
    ubrf12_txstart(data, retlen+2);
}
