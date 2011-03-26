#include <avr/io.h>
#include <stdint.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <string.h>
#include <avr/wdt.h>

#include "ub.h"
#include "ubaddress.h"
#include "ubpacket.h"
#include "timer0.h"
#include "usart.h"
#include "adc.h"

uint16_t publishedInputs = 0;
uint16_t currentInputs = 0;
uint16_t currentChannels[8] = {0,0,0,0,0,0,0,0}; 
uint16_t publishedChannels[8] = {0,0,0,0,0,0,0,0}; 

#define OUT_NONE    0
#define OUT_INPUT   1
#define OUT_CHANNELS   2

uint8_t outinput_channel;
uint8_t outinput_state;

uint16_t outchannels_channel;
uint16_t outchannels_value;

uint8_t out = OUT_NONE;

uint16_t getInputs(void)
{
    return PINC & 0x03;
}

void getChannels(void)
{
    currentChannels[4] = adc_getChannel(4);
    currentChannels[5] = adc_getChannel(5);
    currentChannels[6] = adc_getChannel(6);
}

void sendChannels(void)
{
    getChannels();
    if( !out ){
        int i;
        for(i=0; i<8; i++){
            if( abs(currentChannels[i] - publishedChannels[i]) > 5 ){
                outchannels_value = currentChannels[i];
                outchannels_channel = i;
                publishedChannels[i] = currentChannels[i];
                out = OUT_CHANNELS;
                break;
            }
        }
    }
    if( out == OUT_CHANNELS ){
        if( ubpacket_acquireUnsolicited(UB_CLASS_HID) ){
            if( !ubpacket_isUnsolicitedDone() ){
                struct ubpacket_t *p = ubpacket_getSendBuffer();
                p->header.src = ubadr_getAddress();
                p->header.dest = UB_ADDRESS_MASTER;
                p->header.flags = UB_PACKET_UNSOLICITED;
                p->header.class = UB_CLASS_HID;
                p->header.len = 4;
                p->data[0] = 'A';
                p->data[1] = outchannels_channel+'0';
                p->data[2] = publishedChannels[outchannels_channel] >> 8;
                p->data[3] = publishedChannels[outchannels_channel] & 0xFF;
                ubpacket_send();                       
            }else{
                out = OUT_NONE;
                ubpacket_releaseUnsolicited(UB_CLASS_HID);
            }
        }
    }
}
void sendInputs(void)
{
    currentInputs = getInputs();
    if( !out && currentInputs != publishedInputs ){
        int i;
        for(i=0; i<16; i++){
            if( (currentInputs & (1<<i)) != (publishedInputs & (1<<i)) ){
                if( (currentInputs & (1<<i)) )
                    outinput_state = 'b';
                else
                    outinput_state = 'B';
                outinput_channel = i+'0';
                publishedInputs ^= (1<<i);
                out = OUT_INPUT;
                break;
            }
        }
    }
    if( out == OUT_INPUT ){
        if( ubpacket_acquireUnsolicited(UB_CLASS_HID) ){
            if( !ubpacket_isUnsolicitedDone() ){
                struct ubpacket_t *p = ubpacket_getSendBuffer();
                p->header.src = ubadr_getAddress();
                p->header.dest = UB_ADDRESS_MASTER;
                p->header.flags = UB_PACKET_UNSOLICITED;
                p->header.class = UB_CLASS_HID;
                p->header.len = 2;
                p->data[0] = outinput_state;
                p->data[1] = outinput_channel;
                ubpacket_send();                       
            }else{
                out = OUT_NONE;
                ubpacket_releaseUnsolicited(UB_CLASS_HID);
            }
        }
    }
}

int main(void) {
    //DDRA |= (1<<4 | 1<<5 | 1<<6);
    //PORTC |= (1<<0) | (1<<1) | (1<<2);
    int i = 0;
    DDRC ^= (1<<PC5);
    DDRB |= (1<<PB4);
    PORTC |= 0x03;

    wdt_enable(WDTO_2S);
    uart1_init( UART_BAUD_SELECT(115200,F_CPU));
    adc_init();
    //ub_init(UB_SLAVE, UB_RS485, 0);
    ub_init(UB_SLAVE, UB_RF, 0);
    //PORTC ^= (1<<PC5);
    publishedInputs = (~getInputs())&0x07;
    
    sei();
    timer0_init();

    while (1) {
        wdt_reset();
        
        ub_process();
        if( ubpacket_gotPacket() ){
            struct ubpacket_t * out = ubpacket_getSendBuffer();
            struct ubpacket_t * in = ubpacket_getIncomming();
            if( in->header.class == UB_CLASS_HID ){
                if( in->header.len > 0 ) switch( in->data[0] ){
                    case 'S':
                      if( in->header.len > 1 )
                            PORTA |= (1<<(in->data[1]-'0'));
                    break;
                    case 's':
                        if( in->header.len > 1 )
                            PORTA &= ~(1<<(in->data[1]-'0'));
                    break;
                    case 'G':
                        if( in->header.len > 1 ){
                            uint8_t button = in->data[1]-'0';
                            if( getInputs() & (1<<button)){
                                out->data[0] = 'b';
                            }else{
                                out->data[0] = 'B';
                            }
                            out->data[1] = in->data[1];
                            out->header.len = 2;
                        }
                }
                out->header.class = UB_CLASS_HID;
            }
            ubpacket_processed();
        }
        
        if( timebase ){
            timebase = 0;
            sendInputs();
            //sendChannels();
            ub_tick();
            if( currentChannels[4] > 512 ){
                PORTC |= (1<<PC5);
            }else{
                PORTC &= ~(1<<PC5);
            }
            if( i++ == 1000 ){
                i = 0;
                //PORTC ^= (1<<PC5);
            }
        }
    }
}
