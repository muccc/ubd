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

#include <avr/io.h>     // for device register definitions
#include <avr/eeprom.h> // EEPROM access
#include <avr/sleep.h>     // for power-save idle sleep
#include <avr/wdt.h>       // for watchdog timer
#include <avr/interrupt.h> // for cli/sei
#include <util/delay.h>    // for _delay_us


#if defined (__AVR_ATmega88__) || defined(__AVR_ATmega48__) || defined(__AVR_ATmega168__)
// adapt Mega8 names to the newer Mega88 register+bit names

// interrupts
#define SIG_UART_RECV SIG_USART_RECV
#define SIG_UART_DATA SIG_USART_DATA
#define SIG_UART_TRANS SIG_USART_TRANS

// registers with changed names, but same function
#define OCR2 OCR2A
#define TCCR0 TCCR0B
#define UBRRL UBRR0L
#define UCSRA UCSR0A
#define UCSRB UCSR0B
#define UDR UDR0

// registers with changed function
#define GICR EIMSK
#define GIFR EIFR
#define UCSRC UCSR0C

// register bits with changed names, but same function
#define COM21 COM2A1
#define RXCIE RXCIE0
#define RXEN RXEN0
#define TXCIE TXCIE0
#define TXEN TXEN0
#define UDRIE UDRIE0
#define UCSZ0 UCSZ00
#define UCSZ1 UCSZ01
#define USBS USBS0
#define OCF2 OCF2A
#define OCIE2 OCIE2A

// deprecated register bits
#define URSEL USBS // hack to get rid of deprecated URSEL

// interrupt
#define SIG_OUTPUT_COMPARE2 SIG_OUTPUT_COMPARE2A

#elif defined (__AVR_ATmega8__) || defined (__AVR_ATmega16__) 

// use different names for registers which are split in Mega88
#define TCCR2A TCCR2
#define TCCR2B TCCR2
#define TIMSK0 TIMSK
#define TIMSK1 TIMSK
#define TIFR1 TIFR
#define EICRA MCUCR
#define EIMSK GICR
#define EIFR GIFR

#else
#error "Unsupported AVR controller!"
#endif


// reset watchdog timer
inline static void hal_watchdog_reset(void)
{
    wdt_reset();
}


inline static uint8_t hal_eeprom_read_byte (const uint8_t *addr)
{
    return eeprom_read_byte(addr);
}

inline static uint16_t hal_eeprom_read_word (const uint16_t *addr)
{
    return eeprom_read_word(addr);
}

inline static void hal_eeprom_write_word (uint16_t *addr,uint16_t value)
{
    eeprom_write_word(addr, value);
}

inline static void hal_start_adc(void)
{
    ADCSRA = 0xCE; // enable, start, interrupt, prescaler=64
}

// busy loop for foreground delay
inline static void hal_delay_us(const double us)
{
	_delay_us(us);
}

// prepare sleeping
inline static void hal_sleep_enable(void)
{
    sleep_enable();
}

// the actual sleep instruction
inline static void hal_sleep_cpu(void)
{
    sleep_cpu();
}

// prevent inadvertent sleeping
inline static void hal_sleep_disable(void)
{
    sleep_disable();
}


// this debug function allows to hack in some test code for measurements, 
//   when the PROFILE macro is set to call it
inline static void hal_profile(uint8_t event)
{
    return;
    if (event == PF_SLEEP)
    {
        PORTB &= ~_BV(4); // clear LED
    }
    else
    {
        PORTB |= _BV(4); // set LED
    }
}

#endif // #ifndef _HAL_PLATFORM_H

