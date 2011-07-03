#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "rf12config.h"
#include "rf12.h"
#include "uart.h"
#include "leds.h"

struct RF12_stati
{
    unsigned char Rx:1;
    unsigned char Tx:1;
    unsigned char New:1;
};

volatile unsigned char rf12_checkcrc = 0;

#ifdef RF12_UseNoneBlockingCalls
struct RF12_stati RF12_status;
volatile unsigned char RF12_Index = 0;
unsigned char RF12_Data[RF12_DataLength+10];	// +10 == paket overhead
#endif

unsigned int crcUpdate(unsigned int crc, unsigned char serialData)
{
    unsigned int tmp;
    unsigned char j;

	tmp = serialData << 8;
    for (j=0; j<8; j++){
        if((crc^tmp) & 0x8000)
            crc = (crc<<1) ^ 0x1021;
        else
            crc = crc << 1;
        tmp = tmp << 1;
    }
    return crc;
}

#ifdef RF12DEBUGBIN
void putbin(unsigned short d)
{
    unsigned char i;
    for(i=15;i<=15;i--){
        if(d & (1<<i))
            uart1_putc('1');
        else
            uart1_putc('0');
    }
}
#endif

ISR(RF_SIGNAL, ISR_NOBLOCK)
{
    if(RF12_status.Rx){
        if(RF12_Index < RF12_DataLength){
            leds_rx();
            unsigned char d  = rf12_trans(0xB000) & 0x00FF;
            //uart1_putc(d);
            if(RF12_Index == 0 && d > RF12_DataLength)
                d = RF12_DataLength;
            RF12_Data[RF12_Index++]=d;
        }else{
            rf12_trans(0x8208);
            leds_rxend();
            RF12_status.Rx = 0;
        }
        if(RF12_Index >= RF12_Data[0] + 3){		//EOT
            rf12_trans(0x8208);
            leds_rxend();
            RF12_status.Rx = 0;
            RF12_status.New = 1;
        }
    }else if(RF12_status.Tx){
//		uart_puts("acTab");
        rf12_trans(0xB800 | RF12_Data[RF12_Index]);
        if(!RF12_Index){
#ifdef RF12DEBUGBIN
#ifdef RF12DEBUGBIN2
            uart_puts("acIab");
#endif
#endif
            RF12_status.Tx = 0;
            rf12_trans(0x8208);		// TX off
			
            leds_txend();
        }else{
            RF12_Index--;
        }
    }else{
#ifdef RF12DEBUGBIN
        uart1_puts("acDD");
        unsigned short s = rf12_trans(0x0000);			//dummy read
                //TODO: what happend
        putbin(s);
        uart1_puts("ab");
        uart1_puts("acEinterruptab");
        //while(1);
#endif
    }
}


void spi_init(void)
{
    //DDR_SPI |= (1<<BIT_MOSI) | (1<<BIT_SCK) | (1<<BIT_SPI_SS);
//#if F_CPU > 16000000UL
    SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR1);//SPI Master, clk/64
//#else
//    SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);
//#endif
}

unsigned short rf12_trans(unsigned short d)
{
//    cli();
    RF_EIMSK &= ~(1<<RF_EXTINT);          //Don't block other interrupts(pwm)
    uint16_t retval = 0;
    cbi(RF_PORT, CS);
    SPDR = d>>8;
    while(!(SPSR & (1<<SPIF)));
    retval = SPDR<<8;

    SPDR = d & 0xff;
    while(!(SPSR & (1<<SPIF)));

    retval |= SPDR;
    sbi(RF_PORT, CS);
    //sei();
    RF_EIMSK |= (1<<RF_EXTINT);
    return retval;
}


void rf12_init(void)
{
    unsigned char i;

    RF_DDR |= (1<<SDI)|(1<<SCK)|(1<<CS);
    RF_DDR &= ~(1<<SDO);
    RF_PORT |= (1<<CS);
    RF_PORT |= (1<<SDO);
    spi_init();
#ifdef RESET
    volatile unsigned long j;
    RESET_PORT |= (1<<RESET);
    RESET_PORT &= ~(1<<RESET);
    for(j=0;j<900000;j++);
    RESET_PORT |= (1<<RESET);
#endif
    for (i=0; i<100; i++)
        _delay_ms(10);			// wait until POR done

    rf12_trans(0xC0E0);			// AVR CLK: 10MHz
    rf12_trans(0x80D7);			// Enable FIFO
    rf12_trans(0xC2AB);			// Data Filter: internal
    rf12_trans(0xCA81);			// Set FIFO mode
    rf12_trans(0xE000);			// disable wakeuptimer
    rf12_trans(0xC800);			// disable low duty cycle
    rf12_trans(0xC4F7);			// AFC settings: autotuning: -10kHz...+7,5kHz

    rf12_trans(0x0000);

    RF12_status.Rx = 0;
    RF12_status.Tx = 0;
    RF12_status.New = 0;

    RF_IRQDDR &= ~(1<<IRQ);
    RF_IRQPORT |= (1<<IRQ);

    RF_EICR |= RF_EICR_MASK;
    RF_EIMSK |= (1<<RF_EXTINT);
}

void rf12_setbandwidth(unsigned char bandwidth, unsigned char gain, unsigned char drssi)
{
    rf12_trans(0x9400|((bandwidth&7)<<5)|((gain&3)<<3)|(drssi&7));
}

void rf12_setfreq(unsigned short freq)
{	
    if (freq<96)				// 430,2400MHz
        freq=96;
    else if (freq>3903)			// 439,7575MHz
        freq=3903;
    rf12_trans(0xA000|freq);
}

void rf12_setbaud(unsigned short baud)
{
/*
    if (baud<663)
        return;
    if (baud<5400)					// Baudrate= 344827,58621/(R+1)/(1+CS*7)
        rf12_trans(0xC680|((43104/baud)-1));
    else
        rf12_trans(0xC600|((344828UL/baud)-1));
*/
    rf12_trans(0xC600|16);
}

void rf12_setpower(unsigned char power, unsigned char mod)
{	
    rf12_trans(0x9800|(power&7)|((mod&15)<<4));
}

/* none blocking methods */
unsigned char rf12_rxstart(void)
{
    if(RF12_status.New)
        return(1);			//buffer not yet empty
    if(RF12_status.Tx)
        return(2);			//tx in action
    if(RF12_status.Rx)
        return(3);			//rx already in action

    rf12_trans(0x82C8);			// RX on
    rf12_trans(0xCA81);			// set FIFO mode
    rf12_trans(0xCA83);			// enable FIFO
	
    RF12_Index = 0;
    RF12_status.Rx = 1;
    return(0);				//all went fine
}

unsigned char rf12_rxfinish(unsigned char *data)
{
	unsigned int crc, crc_chk = 0;
	unsigned char i;
	if(RF12_status.Rx)
		return(255);				//not finished yet
	if(!RF12_status.New)
		return(254);				//old buffer

    RF12_status.New = 0;
    if(rf12_checkcrc){
        for(i=0; i<RF12_Data[0] +1 ; i++)
            crc_chk = crcUpdate(crc_chk, RF12_Data[i]);

        crc = RF12_Data[i++];
        crc |= RF12_Data[i] << 8;
        if(crc != crc_chk)
            return(0);				//crc err -or- strsize
    }
    for(i=0; i<RF12_Data[0]; i++)
        data[i] = RF12_Data[i+1];
    return(RF12_Data[0]);			//strsize
}

unsigned char rf12_txstart(unsigned char *data, unsigned char size)
{	
#ifdef RF12DEBUGBIN
#ifdef RF12DEBUGBIN2
uart_puts("acbab");
#endif
#endif
    leds_tx();
    unsigned char i, l;
    unsigned int crc;
    if(RF12_status.Tx)
        return(2);          //tx in action
    //if(RF12_status.Rx)
        //return(3);          //rx already in action
    if(size > RF12_DataLength)
        return(4);          //str to big to transmit
	
    RF12_status.Tx = 1;
    RF12_Index = size + 9;      //act -10 

    i = RF12_Index;				
    RF12_Data[i--] = 0xAA;
    RF12_Data[i--] = 0xAA;
    RF12_Data[i--] = 0xAA;
    RF12_Data[i--] = 0x2D;
    RF12_Data[i--] = 0xD4;
    RF12_Data[i--] = size;
    crc = crcUpdate(0, size);
    for(l=0; l<size; l++){
        RF12_Data[i--] = data[l];
        crc = crcUpdate(crc, data[l]);
    }
    RF12_Data[i--] = (crc & 0x00FF);
    RF12_Data[i--] = (crc >> 8);
    RF12_Data[i--] = 0xAA;
    RF12_Data[i--] = 0xAA;

    rf12_trans(0x8238);         // TX on
#ifdef RF12DEBUGBIN
#ifdef RF12DEBUGBIN2
    uart_puts("accab");
#endif
#endif
    return(0);              //all went fine
}

unsigned char rf12_txfinished(void)
{
    if(RF12_status.Tx){
#ifdef RF12DEBUGBIN
#ifdef RF12DEBUGBIN2
        uart_puts("actab");;
#endif
#endif
        return(255);        //not yet finished
    }
    return(0);
}

void rf12_allstop(void)
{
    rf12_trans(0x8208);     //shutdown all
    RF12_status.Rx = 0;
    RF12_status.Tx = 0;
    RF12_status.New = 0;
    rf12_trans(0x0000);     //dummy read
}
/* ---------------------- */
