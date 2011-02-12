#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <string.h>

#include "config.h"
#include "leds.h"
#include "flash.h"
#include "ubconfig.h"
#include "ubcrc.h"
#include "ubrs485message.h"
#include "uart.h"
#include "pinutils.h"

#define CHUNKSIZE       32
#define UBBOOTMAGIC     "UBBOOTLOADER"

uint8_t pagedata[SPM_PAGESIZE];
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

void tx(uint8_t c)
{
    PIN_SET(RS485_TX_PIN);
    PIN_SET(RS485_NRX_PIN);
    UDR0 = c;
    while( !(UCSR0A & (1<<TXC0)) );
    UCSR0A |= (1<<TXC0);
    PIN_CLEAR(RS485_TX_PIN);
    PIN_CLEAR(RS485_NRX_PIN);
}

void sendresult(uint8_t * data, uint8_t len)
{
    uint8_t i;
    tx('\\');
    tx('5');
    for(i=0;i<len;i++){
        tx(data[i]);
    }
    tx('\\');
    tx('2');
    }

int main(void)
{
    uint8_t tmp = MCUCR;
    MCUCR = tmp | (1<<IVCE);
    MCUCR = tmp | (1<<IVSEL);

    leds_init();

    rs485msg_init();
    DDR_CONFIG_OUT(RS485_TX_PIN);
    DDR_CONFIG_OUT(RS485_NRX_PIN);

    PIN_CLEAR(RS485_TX_PIN);
    PIN_CLEAR(RS485_NRX_PIN);
    DDRA = 0xFF;
    PORTA = 0x1;
    UBRR0 = UART_BAUD_SELECT(RS485_BITRATE,F_CPU);
    UCSR0B |= (1<<TXEN0) | (1<<RXEN0);
    wdt_enable(WDTO_2S);
    while(1){
        wdt_reset();
        if( (UCSR0A & (1<<RXC0)) ){
            uint8_t c = UDR0;
            uint8_t type = rs485msg_put(c);
            if( type == UB_BOOTLOADER ){
                uint16_t len = rs485msg_getLen();
                uint8_t *data = rs485msg_getMsg();

                if(len < 3)
                    continue;

                uint16_t crc = ubcrc16_data(data, len-2);
                if( data[len-2] == ((crc>>8) & 0xFF) &&
                    data[len-1] == (crc&0xFF) ){
                    cmd(data[0],data+1,len-3);    
                }else{
                    tx('c');
                    tx(crc>>8);
                    tx(crc&0xFF);
                }
            }
        }
       if( !selected && timeout++ > 1000000UL ){

            PORTA = 0x4;
            boot();
       }
    }
}

static uint8_t compare_flash(uint16_t start, uint8_t len, uint8_t *data)
{
    return 1;
}

static uint8_t compare_eeprom(uint16_t start, uint8_t len, uint8_t *data)
{
    uint8_t buf[100];
    eeprom_read_block(buf,(uint8_t*)start,len);
    if( memcmp(buf,data,len)==0 )
        return 1;
    return 0;
}

static void cmd(uint8_t cmd, uint8_t *data, uint16_t len)
{
    uint8_t ret = 'f';
    uint8_t retlen = 0;
    static uint8_t booted = 0;
    if( cmd == 'B' ){
        data[len]=0;
        if( strcmp((char*)data,UBBOOTMAGIC)==0 ){
            booted = 1;
        }
        return;
    }
    uint16_t start;
    if( cmd == 'S' && booted ){
        selected = 0;
        start = (data[1]<<8)|data[2];
        switch( data[0] ){
            case 'F':
                if( compare_flash(start, len-3, data+3) )
                    selected = 1;
            break;
            case 'E':
                if( compare_eeprom(start, len-3, data+3) )
                    selected = 1;
            break;
        }
        if( selected == 1 ){
            ret = 'S';
            PORTA = 0x2;
        }
    }

    if( !selected )
        return;

    switch(cmd){
        case 'P':
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
            start = (data[0]<<8)|data[1];
            len = data[2];
            eeprom_read_block(data,(uint8_t*)start,len);
            retlen = len;
        break;
        case 'E':
            start = (data[0]<<8)|data[1];
            eeprom_write_block(data+2,(uint8_t*)start,len-2);
            ret = cmd;
        break;
        case 'G':
            boot();
        break;
    }

    if( retlen )
        sendresult(data-1,retlen+1);
    else
        sendresult(&ret,1);
}

