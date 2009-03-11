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

/*! \file uart_platform.h
    \brief UART hardware abstraction, Atmel Mega8 part.
*/

#ifndef _UART_ATMEL_H
#define _UART_ATMEL_H

#include <avr/io.h>     // for device register definitions
#include <avr/interrupt.h> // for interrupt handlers

#define UART_RX_ISR         SIGNAL(SIG_UART_RECV)

#define BAUDRATE 19200ULL // ULL for being 32 bit with -mint8

__attribute__((always_inline)) static void hal_uart_rs485_disable(void);
inline static void hal_uart_rs485_disable(void) 
{
    PORTC &= ~_BV(PC5);
}

inline static void hal_uart_rs485_enable(void) 
{
	PORTC |= _BV(PC5); // enable the RS485 driver
}

inline static void hal_uart_init_hardware(void)
{
    UBRRL = (uint8_t)(F_CPU / (16*BAUDRATE) - 1); // 11
    UCSRB = _BV(RXCIE) | _BV(RXEN) | _BV(TXEN); // enable RX+TX with RX interrupt
    UCSRC = _BV(URSEL) | _BV(USBS) | _BV(UCSZ1) | _BV(UCSZ0); // 8N2
}

inline static uint8_t hal_uart_has_errors(void)
{   // no need to check for errors, since we don't have to clear them
    return 0; // UCSRA & 0x1C;
}

inline static void hal_uart_clear_errors(void)
{   // not necessary on AVR, cleared when reading data
}

inline static uint8_t hal_uart_has_data(void)
{   // within interrupt, it always has data
    return 1; // UCSRA & _BV(RXC);
}

inline static uint8_t hal_uart_read_data(void)
{
    return UDR;
}

inline static void hal_uart_send_data(uint8_t b)
{
    UDR = b;
}


#endif // #ifndef _UART_ATMEL_H
