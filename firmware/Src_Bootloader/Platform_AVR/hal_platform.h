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

/*! \file hal_platform.h
    \brief Hardware abstraction layer, AVR inline implementations.
*/

#ifndef __AVR
#error "Wrong include path, this is for Atmel AVR only!"
#endif

#ifndef _HAL_PLATFORM_H
#define _HAL_PLATFORM_H


#include <stdint.h>   // common scalar types
#include <avr/io.h>     // for device register definitions
#include <avr/interrupt.h> // for interrupt enable
#include <avr/sleep.h>  // for power-save idle sleep
#include <avr/eeprom.h> // for EEPROM access
#include <avr/boot.h> // for flash programming
#include <avr/pgmspace.h> // for flash read access
#include <util/delay.h> // for delay loops
#include "hal.h"


#if defined (__AVR_ATmega88__) || defined(__AVR_ATmega48__) || defined(__AVR_ATmega168__)
// adapt Mega8 names to the newer Mega88 register+bit names

// interrupts
#define SIG_UART_RECV SIG_USART_RECV

// registers with changed names, but same function
#define OCR2 OCR2A
#define UBRRL UBRR0L
#define UCSRA UCSR0A
#define UCSRB UCSR0B
#define UDR UDR0
#define TCCR0 TCCR0B

// registers with changed function
#define GICR MCUCR
#define UCSRC UCSR0C

// register bits  with changed names, but same function
#define RXCIE RXCIE0
#define RXEN RXEN0
#define TXEN TXEN0
#define UCSZ1 UCSZ01
#define UCSZ0 UCSZ00
#define USBS USBS0
#define OCF2 OCF2A

// deprecated register bits
#define URSEL USBS // hack to get rid of deprecated URSEL

#elif defined (__AVR_ATmega8__)
// use different names for registers which are split in Mega88
#define TIMSK0 TIMSK
#define TIFR0 TIFR
#define TIFR2 TIFR
#define TCCR2B TCCR2

#else
#error "Unsupported AVR controller!"
#endif

#define HZ 56L // IRQ ticks per second
#define TIMER_FREQ (F_CPU/256L) // timer frequency, 14400L

#define TIMER_TICK_ISR    SIGNAL(SIG_OVERFLOW0)

// timeouts triggered by byte reception
#define RX_TIMEOUT 2 // timeout for non-continuous receive, packet end
#define TURNAROUND 4 // time from receiving to sending, the fastest original module I've seen replies 4 bits after STM frame


#ifdef DEBUG
inline static void hal_assert(const char* file, uint16_t line)
{
	hal_panic();
}
#endif

inline static void hal_sysinit(void)
{
    GICR = _BV(IVCE); // Enable interrupt vector select
    GICR = _BV(IVSEL); // Move interrupt vector to bootloader

    // DDRD = 0x00; // all input, should be default
    PORTD = 0xF8; // pullups on DIP switch, do this early to allow settling

    DDRC =  _BV(4) | _BV(5); // output channel and RS485 data enable
#ifdef DEBUG
	DDRB =  _BV(4) | _BV(5); // aux outputs on ISP connector
#endif
    
    // timer 0 init, tick interrupt
    TCCR0 = 0x04; // prescale with 256 -> Clk/65536 overflows per sec = 56.25 Hz
    TIMSK0 = _BV(TOIE0); // enable interrupt

    // timer 2 init, receive timeouts (no interrupt)
    TCCR2B = 0x04; // prescale with 64 -> one UART bit is 3 timer ticks
    // first, the overflow will happen to generate timeout1 (turnaround)
    // second, the match will happen to generate timeout2 (rx timeout)
    OCR2 = (1+8+2+RX_TIMEOUT - (1+TURNAROUND))*3; // set to remain after wrap: timeout2 - timeout1

    sleep_enable(); // globally enable sleep, here I don't bother for tight en/disable

    sei(); // enable interrupts
}

// busy loop for foreground delay
// (wrapper function doesn't work here, it will do the float calc at runtime when called more than once)
#define hal_delay_us(us) _delay_us(us);


// set the CPU into sleep mode, until next interrupt
inline static void hal_sleep_cpu(void)
{
    sleep_cpu(); // the actual sleep instruction
}

// signal a fatal error
#ifdef DEBUG
inline static void hal_panic(void)
{
    cli(); // disable interrupts
	while (1)
	{	// wild flashing LEDs
		_delay_loop_2(20000);
		PORTB ^= _BV(4) | _BV(5); // aux outputs on ISP connector
		PORTC ^= _BV(4); // output channel
	}
}
#endif

// read the module address from the DIP switches
inline static uint8_t hal_get_addr(void)
{
    uint8_t val;
    
    val = ~PIND; // read PD3...PD7, invert
    val >>= 3;

    PORTD = 0x00; // optional: set low, to save a little on power (0.1mA per closed switch)
    DDRD = 0xF8; // output
    // in consequence, we can't call this again without port re-init to input

    return val;
}

// set the feedback LEDs
#ifdef DEBUG
/*
inline static void hal_set_led(uint8_t leds) // UEM hardware
{
    PORTC = (PORTC & 0xEF) | ((leds & 0x01) << 4); // set bit 0 to PC4
    PORTB = (PORTB & 0xCF) | ((leds & 0x06) << 3); // set bit 1+2 to PC4+PC5
}
#endif
*/
/*
inline static void hal_set_led(uint8_t leds) // HSM hardware
{
    // test hack!!
    DDRC |= 0x1F; // output bits 0...4
    DDRB |= 0x07; // output bits 5...7
    PORTB = (PORTB & 0xF8) | (leds & 0x07); // Bit 0...2: PB0...PB2
    PORTC = (PORTC & 0xE0) | ((leds & 0xF8) >> 3); // Bit 3...8: PC0...PC4
}
*/
#endif

inline static void hal_timer_clear_irq(void)
{
    // nothing necessary on AVR
}


// use timer2 for two subsequent timeouts
// if code size is critical, unify to only one timeout, run from 0x00 to OCR2

inline static void hal_timeout_respawn(void)
{
    TCNT2 = 256 - (1 + TURNAROUND)*3; // 2nd stopbit plus 1st timeout until wrap
    TIFR2 = _BV(OCF2) | _BV(TOV2); // clear the compare and overflow flags
}

inline static uint8_t hal_timeout1_test(void)
{
    return TIFR2 & _BV(TOV2); // return nonzero if overflow happened
}

inline static uint8_t hal_timeout2_test(void)
{
    return TIFR2 & _BV(OCF2); // return nonzero if compare match happened
}


// copy 16 byte into the temporary buffer, starting at "address"
inline static void hal_flash_load(uint16_t* buf, uint8_t address, uint8_t len)
{
    uint8_t i;

    for (i=0; i<len; i+=2)
    {
        // exploit that we're little endian
        uint16_t w = *buf++;
        cli();
        __boot_page_fill_normal(address + i, w);
        sei();
    }
}

inline static void hal_flash_write(uint16_t page)
{
    page *= SPM_PAGESIZE;

    cli();
    __boot_page_erase_normal(page);
    sei();
    boot_spm_busy_wait(); // wait until the memory is erased

    cli();
    __boot_page_write_normal(page); // store buffer in flash page
    sei();
    boot_spm_busy_wait(); // wait until the memory is written

    cli();
    __boot_rww_enable(); // allow reading the RWW section again
    sei();
}

inline static void hal_flash_read (
    void *pointer_ram,
    uint16_t index_flash,
    uint8_t n)
{
    memcpy_P(pointer_ram, (void*)index_flash, n);
}

inline static void hal_eeprom_read (
    void *pointer_ram,
    uint16_t index_eeprom,
    uint8_t n)
{
    eeprom_read_block(pointer_ram, (void*)index_eeprom, n);
}

inline static void hal_eeprom_write (
    const void *pointer_ram,
    uint16_t index_eeprom,
    uint8_t n)
{
    eeprom_write_block(pointer_ram, (void*)index_eeprom, n);
}


__attribute__((noinline)) static void hal_reboot_app(void);
static void hal_reboot_app(void)
{
    cli();

    GICR = _BV(IVCE); // enable interrupt vector select
    GICR = 0x00; // move interrupt vector to app flash

    // clean up
    sleep_disable(); // better no more global sleep enable
    TIMSK0 = 0x00; // disable timer interrupts

    // if codespace gets tight, the optional parts have to be done by app
    DDRD = 0x00; // optional: inputs for DIP switches again
#ifndef SAVE_CODESPACE
    UCSRB = 0x00; // (really optional, app will use UART) disable UART interrupts and pins
#endif
    // push zero as the return address on the stack
    asm volatile ("push r1\n\t" // run application code
                  "push r1\n\t" ::);
}


#endif // #ifndef _HAL_PLATFORM_H

