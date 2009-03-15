/***************************************************************************
 *
 * OpenHC:                          ___                 _  _  ___
 *  Open source                    / _ \ _ __  ___ _ _ | || |/ __|
 *  Home                          | (_) | '_ \/ -_) ' \| __ | (__ 
 *  Control                        \___/| .__/\___|_||_|_||_|\___|
 * http://openhc.sourceforge.net/       |_| 
 *
 * Copyright (C) 2005 by Joerg Hohensohn
 *
 * All files in this archive are subject to the GNU General Public License.
 * See http://www.gnu.org/licenses/gpl-3.0.txt for full license agreement.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ****************************************************************************/ 

/*! \file hal_atmel.c
    \brief Hardware abstraction layer, Atmel AVR implementation.
*/

#define __FILENUM__ 2 // every file needs to have a unique (8bit) ID

#include <stdint.h>        // common scalar types
#include <avr/io.h>        // for device register definitions
//#include <avr/signal.h>    // for interrupt handlers
#include <avr/interrupt.h> // for interrupt enable
#include <avr/sleep.h>     // for power-save idle sleep
#include <avr/wdt.h>       // for watchdog timer
#include "hal.h"
#include "errlog.h"


// which hardware is this for, from the CFG_xx setting in makefile
// (ToDo: perhaps set the IS_xx in the makefile and derive CFG_xx from it, in .h file)
#if defined(CFG_IR)
#define IS_UEM // infrared receiver with UEM hardware
#elif defined(CFG_INPUT) && CFG_INPUT==8 // hardware for UEM
#define IS_UEM // 8 inputs, UEM layout
#elif defined(CFG_INPUT) && CFG_INPUT==16 // hardware for EM16
#define IS_EM16 // 16 inputs, HSM layout
#elif defined(CFG_OUTPUT)
#define IS_AM // 8 outputs, HSM layout
#elif defined(CFG_BRIDGE)
#define IS_BRIDGE // 8 outputs, HSM layout
#elif defined(CFG_SENDER)
#define IS_SENDER
#else
#error No hardware setup defined!
#endif



#ifdef CFG_ADC
static uint16_t hal_adc_val; // a place for the last ADC readout
#endif

#ifdef DEBUG // verbose, with filename
void hal_assert(const char* file, uint16_t line)
{
	hal_panic();
}
#else // log only a file number
void hal_assert(uint8_t filenum, uint16_t line)
{
	// hal_set_led(0xFF); this was convenient for debugging
//    errlog(filenum, line);
}
#endif

void hal_sysinit(void)
{
    // early port init for DIP switches, so the pullups have time to settle
    DDRD &= 0x07; // input, the bootloader may have left them as output
    PORTD |= 0xF8; // pullups on DIP switch
#if defined(IS_SENDER)
    DDRC =  _BV(PC4) | _BV(PC5) | _BV(PC3);
#endif

#if defined(IS_UEM) // hardware for UEM
    DDRC =  _BV(4) | _BV(5); // output channel and RS485 data enable
	DDRB =  _BV(4) | _BV(5); // aux outputs on ISP connector
    //PORTB |= 0x0F; // internal pullups for input (only necessary for breadboard)
    //PORTC |= 0x0F; // internal pullups for input (only necessary for breadboard)
#elif defined(IS_EM16) // hardware for EM16
	DDRC = _BV(5); // RS485 data enable
	DDRB = _BV(3) | _BV(4) | _BV(5); // select, aux outputs on ISP connector
    //PORTB |= 0x07; // internal pullups for input (only necessary for breadboard)
    //PORTC |= 0x1F; // internal pullups for input (only necessary for breadboard)
#elif defined(IS_AM)
	DDRC = 0x1F | _BV(5); // output bits 0...4 and RS485 data enable
	DDRB = 0x07 | _BV(3) | _BV(4) | _BV(5); // output bits 5...7, PWM, aux outputs on ISP connector
#elif defined(IS_BRIDGE)
    DDRC |= (1<<PC5) | (1<<PC4);
    PORTC &= (1<<PC5);
#endif

#ifdef CFG_ADC
    ADMUX = 0xC7; // measure channel 7, internal 2.56V reference
#endif

#if defined (__AVR_ATmega644P__)
    EICRA = _BV(ISC21); // interrupt on falling edge of INT0 
    EIMSK |= _BV(INT2); // enable INT0
#else
    EICRA = _BV(ISC01); // interrupt on falling edge of INT0 
    EIMSK |= _BV(INT0); // enable INT0
#endif
    sei(); // globally enable interrupts
}

// set the CPU into sleep mode, until next interrupt
void hal_suspend(void)
{
    PROFILE(PF_SLEEP);
    sleep_mode();
}

// signal a fatal error
//#ifdef DEBUG
void hal_panic(void)
{
    cli(); // disable interrupts
	while (1)
	{	// wild flashing LEDs
		volatile uint16_t i;
		for (i=0; i<20000; i++);
		PORTB ^= _BV(4) | _BV(5); // aux outputs on ISP connector
#ifdef IS_UEM // this has one more
		PORTC ^= _BV(4); // output channel
#endif
	}
}
//#endif

// enable watchdog timer
void hal_watchdog_enable()
{
    wdt_enable(WDTO_500MS/*WDTO_2S*/); // ToDo: tailor this to minimum value, = UART timeout?
}

void hal_reboot(void)
{
    wdt_enable(WDTO_15MS); // abuse watchdog: set to minimum
    while (1) hal_suspend(); // and let it trigger
}

// read the module address from the DIP switches
uint8_t hal_get_addr(void)
{
    uint8_t val;
    
    val = ~PIND; // read PD3...PD7, invert
    val >>= 3;

    PORTD &= 0x07; // set low, to save a little on power (0.1mA per closed switch)
    DDRD |= 0xF8; // output
    // in consequence, we can't call this again without port re-init to input

    return val;
}

// set the feedback LEDs
void hal_set_led(uint8_t leds)
{
#if defined (IS_UEM)
    PORTC = (PORTC & 0xEF) | ((leds & 0x01) << 4); // set bit 0 to PC4
    PORTB = (PORTB & 0xCF) | ((leds & 0x06) << 3); // set bit 1+2 to PB4+PB5
#elif defined (IS_EM16) || defined (IS_AM) // HSM layout
    PORTB = (PORTB & 0xCF) | ((leds & 0x03) << 4); // set bit 0+1 to PB4+PB5
#endif
}

#ifdef CFG_OUTPUT
// set the outputs
void hal_set_output(uint8_t byte)
{
    PORTB = (PORTB & 0xF8) | (byte & 0x07); // Bit 0...2: PB0...PB2
    PORTC = (PORTC & 0xE0) | ((byte & 0xF8) >> 3); // Bit 3...8: PC0...PC4
}
#endif

// read the input switches
#ifdef CFG_INPUT
uint16_t hal_get_switch(void)
{
#if defined(IS_UEM)
    return  ~((PINB & 0x0F) | ((PINC & 0x0F) << 4)); // read from PB and PC, invert
#elif defined(IS_EM16)
    // this one has multiplexed inputs
    uint16_t retval;
    unsigned mask8;
    uint16_t mask16;
    uint8_t ret_h, ret_l; // both bank's readout
    uint8_t bank = PORTB; // raw port, for now

    ret_l = (PINB & 0x07) | ((PINC & 0x1F) << 3); // read from PB and PC
    PORTB = bank ^ _BV(3); // flip the select over to other bank
    __asm__ __volatile__ ("nop\n nop\n nop\n nop\n"); // let the inputs settle for a while
    ret_h = (PINB & 0x07) | ((PINC & 0x1F) << 3); // read from PB and PC

    if (bank & _BV(3)) // select was high at first?
    {   // swap high and low part
        uint8_t temp = ret_l;
        ret_l = ret_h;
        ret_h = temp;
    }

    // interleave the high and low bits for proper pinout
    retval = 0;
    mask8=0x80; // "single speed" mask for 8 bit readout
    mask16=0x8000; // "double speed" mask for 16 bit result
    for (; mask8; mask8 >>= 1)
    {
        if (ret_h & mask8) retval |= mask16;
        mask16 >>= 1;
        if (ret_l & mask8) retval |= mask16;
        mask16 >>= 1;
    }

    return  ~retval; // inverted because switches pull down
#else
    #error No input hardware defined!
#endif
}
#endif


#ifdef CFG_OUTPUT
// pulsed PWM output to save relay power
// byte=255 is DC high, byte=0 is DC low
void hal_set_pwm(uint8_t byte)
{
    return;
    if (byte != 0)
    {
        // fast PWM, clear OC2 on match, clk/1 -> 14400 wraps/sec
		TCCR2A = _BV(WGM21) | _BV(WGM20) | _BV(COM21) | _BV(CS20); 
		TCCR2B |= _BV(CS20); // note: same as TCCR2A on Mega8
        OCR2 = byte;
    }
    else // special case: instead of minimum needle pulse, we want 0V DC
    {
        TCCR2A = 0x00;
    }
}
#endif

#ifdef CFG_ADC
// return the measured supply voltage (in mV)
uint16_t hal_read_voltage(void)
{
    return hal_adc_val * (uint16_t)40; // external resistor divider = 1/16
}


SIGNAL(SIG_ADC)
{
    uint16_t result;
    PROFILE(PF_ISRSTART_ADC);
    result = ADC; // compiler does low/high read sequence
    hal_adc_val = result; // remember the value in a static var
    ADCSRA = 0; // turn ADC off again
    PROFILE(PF_ISREND_ADC);
}

// start an ADC conversion
/* inlined
void hal_start_adc(void)
{
    ADCSRA = 0xCE; // enable, start, interrupt, prescaler=64
}
*/
#endif // CFG_ADC


#ifdef CFG_IR
// read the timer capture
uint16_t hal_get_capture(void)
{
    uint16_t result;
    result = ICR1; // compiler does low/high read sequence
	return result;
}

// query the capture edge, returns 0=falling, 1=rising
uint8_t hal_get_edge(void)
{
	uint8_t value = TCCR1B;
	return (value & _BV(ICES1)) >> ICES1; // return edge setting
}

// set the capture edge, 0=falling, 1=rising
void hal_set_edge(uint8_t edge)
{
	uint8_t value = TCCR1B;

    if (edge)
		value |= _BV(ICES1); // set for rising
    else
        value &= ~_BV(ICES1); // clear for falling

	TCCR1B = value;
}
#endif // CFG_IR
