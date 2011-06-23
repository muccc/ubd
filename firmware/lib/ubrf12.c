#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include "ubconfig.h"
#include "ubrf12.h"
#include "ubleds.h"

struct RF12_stati
{
    unsigned char Rx:1;
    unsigned char Tx:1;
    unsigned char New:1;
};

struct RF12_stati RF12_status;
volatile unsigned char RF12_Index = 0;
unsigned char RF12_Data[RF12_DataLength+10];	// +10 == packet overhead
unsigned char RF12_channel;

ISR(RF_SIGNAL, ISR_NOBLOCK)
{
    if(RF12_status.Rx){
        if(RF12_Index < RF12_DataLength){
            ubleds_rx();
            unsigned char d  = ubrf12_trans(0xB000) & 0x00FF;
            if(RF12_Index == 0 && d > RF12_DataLength)
                d = RF12_DataLength;
            RF12_Data[RF12_Index++]=d;
        }else{
            ubrf12_trans(0x8208);
            ubleds_rxend();
            RF12_status.Rx = 0;
        }
        if(RF12_Index >= RF12_Data[0] + 3){		//EOT
            ubrf12_trans(0x8208);
            ubleds_rxend();
            RF12_status.Rx = 0;
            RF12_status.New = 1;
        }
    }else if(RF12_status.Tx){
        ubrf12_trans(0xB800 | RF12_Data[RF12_Index]);
        if(!RF12_Index){
            RF12_status.Tx = 0;
            ubrf12_trans(0x8208);		// TX off
            ubleds_txend();
        }else{
            RF12_Index--;
        }
    }else{
    }
}

void spi_init(void)
{
    SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR1);//SPI Master, clk/64
}

unsigned short ubrf12_trans(unsigned short d)
{
    //Don't block other interrupts(pwm)
    uint8_t eimsk = RF_EIMSK;
    RF_EIMSK &= ~(1<<RF_EXTINT);

    uint16_t retval = 0;
    cbi(RF_PORT, CS);
    SPDR = d>>8;
    while(!(SPSR & (1<<SPIF)));
    retval = SPDR<<8;

    SPDR = d & 0xff;
    while(!(SPSR & (1<<SPIF)));

    retval |= SPDR;
    sbi(RF_PORT, CS);

    RF_EIMSK = eimsk;
    return retval;
}


void ubrf12_init(unsigned char channel)
{
    unsigned char i;
    RF12_channel = channel;

    RF_DDR |= (1<<SDI)|(1<<SCK)|(1<<CS);
    RF_DDR &= ~(1<<SDO);
    RF_PORT |= (1<<CS);
    RF_PORT |= (1<<SDO);
    spi_init();
#ifdef RESET
    volatile unsigned long j;
    RESET_DDR |= (1<<RESET);
    RESET_PORT |= (1<<RESET);
    _delay_ms(10);
    RESET_PORT &= ~(1<<RESET);
    for(j=0;j<900000;j++)
        wdt_reset();
    RESET_PORT |= (1<<RESET);
#endif
    for (i=0; i<100; i++){
        _delay_ms(10);			// wait until POR done
        wdt_reset();
    }

    ubrf12_trans(0xC0E0);			// AVR CLK: 10MHz
    ubrf12_trans(0x80D7);			// Enable FIFO
    ubrf12_trans(0xC2AB);			// Data Filter: internal
    ubrf12_trans(0xCA81);			// Set FIFO mode
    ubrf12_trans(0xE000);			// disable wakeuptimer
    ubrf12_trans(0xC800);			// disable low duty cycle
    ubrf12_trans(0xC4F7);			// AFC settings: autotuning: -10kHz...+7,5kHz

    ubrf12_trans(0x0000);

    RF12_status.Rx = 0;
    RF12_status.Tx = 0;
    RF12_status.New = 0;

    RF_IRQDDR &= ~(1<<IRQ);
    RF_IRQPORT |= (1<<IRQ);

    RF_EICR |= RF_EICR_MASK;
    RF_EIMSK |= (1<<RF_EXTINT);
}

void ubrf12_setbandwidth(unsigned char bandwidth, unsigned char gain, unsigned char drssi)
{
    ubrf12_trans(0x9400|((bandwidth&7)<<5)|((gain&3)<<3)|(drssi&7));
}

void ubrf12_setfreq(unsigned short freq)
{	
    if (freq<96)				// 430,2400MHz
        freq=96;
    else if (freq>3903)			// 439,7575MHz
        freq=3903;
    ubrf12_trans(0xA000|freq);
}

void ubrf12_setbaud(unsigned short baud)
{
/*    if (baud<663)
        return;
    if (baud<5400)					// Baudrate= 344827,58621/(R+1)/(1+CS*7)
        ubrf12_trans(0xC680|((43104/baud)-1));
    else
        ubrf12_trans(0xC600|((344828UL/baud)-1));*/
    //19200:
    ubrf12_trans(0xC600|16);
}

void ubrf12_setpower(unsigned char power, unsigned char mod)
{	
    ubrf12_trans(0x9800|(power&7)|((mod&15)<<4));
}

unsigned char ubrf12_rxstart(void)
{
    if(RF12_status.New)
        return(1);			//buffer not yet empty
    if(RF12_status.Tx)
        return(2);			//tx in action
    if(RF12_status.Rx)
        return(3);			//rx already in action

    ubrf12_trans(0x82C8);			// RX on
    ubrf12_trans(0xCA81);			// set FIFO mode
    ubrf12_trans(0xCA83);			// enable FIFO
	
    RF12_Index = 0;
    RF12_status.Rx = 1;
    return(0);				//all went fine
}

unsigned char ubrf12_rxfinish(unsigned char *data)
{
    unsigned char i;
    //not finished yet or old buffer
    if( RF12_status.Rx || !RF12_status.New ){
        return 255;
    }
    RF12_status.New = 0;

    if( RF12_Data[RF12_Data[0]+1] != RF12_channel ){
        return 255;
    }
    
    for(i=0; i<(RF12_Data[0]); i++)
        data[i] = RF12_Data[i+1];
    return(RF12_Data[0]);			//strsize
}

void ubrf12_txstart(unsigned char *data, unsigned char size)
{	
    ubleds_tx();
    uint8_t i, l;

    ubrf12_allstop();
    if(size > RF12_DataLength)
        return;          //to big to transmit, ignore
	
    RF12_status.Tx = 1;
    RF12_Index = i = size + 9;      //act -10 

    RF12_Data[i--] = 0xAA;
    RF12_Data[i--] = 0xAA;
    RF12_Data[i--] = 0xAA;
    RF12_Data[i--] = 0x2D;
    RF12_Data[i--] = 0xD4;
    RF12_Data[i--] = size;
    for(l=0; l<size; l++){
        RF12_Data[i--] = data[l];
    }
    //TODO: remove these two bytes
    RF12_Data[i--] = RF12_channel;
    RF12_Data[i--] = 0;
    RF12_Data[i--] = 0xAA;
    RF12_Data[i--] = 0xAA;

    ubrf12_trans(0x8238);         // TX on
    return;
}

unsigned char ubrf12_txfinished(void)
{
    if(RF12_status.Tx){
        return 0;        //not yet finished
    }
    return 1;
}

void ubrf12_allstop(void)
{
    ubrf12_trans(0x8208);     //shutdown all
    RF12_status.Rx = 0;
    RF12_status.Tx = 0;
    RF12_status.New = 0;
    ubrf12_trans(0x0000);     //dummy read
}

uint8_t ubrf12_free(void)
{
    if( (ubrf12_trans(0x0000)>>8) & (1<<0) )
        return 0;
    return 1;
}

