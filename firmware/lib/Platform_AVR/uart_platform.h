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
    \brief UART hardware abstraction, Atmel Mega8 part.
*/

#ifndef __AVR
#error "Wrong include path, this is for Atmel AVR only!"
#endif

#ifndef _UART_PLATFORM_H
#define _UART_PLATFORM_H

#include <avr/io.h>     // for device register definitions
#include <avr/interrupt.h> // for interrupt handlers

#define UART_RX_ISR          SIGNAL(SIG_UART_RECV)
#define UART_TX_ISR          SIGNAL(SIG_UART_DATA)
#define UART_TX_DONE_ISR     SIGNAL(SIG_UART_TRANS)
#define UART_RX_TIMEOUT_ISR  SIGNAL(SIG_OUTPUT_COMPARE1A)
#define UART_RX_TIMEOUT2_ISR SIGNAL(SIG_OUTPUT_COMPARE1B)
#define UART_RX_EDGE_ISR     SIGNAL(SIG_INTERRUPT0)

#define BAUDRATE 19200ULL // ULL for being 32 bit with -mint8

// this timeout also controls our answer time, to allow for STM turnaround
#define RX_TIMEOUT (1+8+2 + 5) // allow byte time 5 bits grace time between bytes (so slow because of fat ISRs?)

// consider the line as free after waiting for some worst case answer time
#define FREELINE_TIME (1+33) // free after no less than 33 bits idle, plus 2nd stopbit


// static variables
static uint16_t uart_timeout; // receive timeout, when do we dare start sending


inline static void hal_uart_rs485_disable(void) 
{
    PORTC &= ~_BV(PC5); 
}

inline static void hal_uart_rs485_enable(void) 
{
	PORTC |= _BV(PC5); // enable the RS485 driver
}

inline static void hal_uart_rx_edge_disable(void) 
{
    EIMSK &= ~_BV(INT0); // disable INT0
}

inline static void hal_uart_rx_edge_enable(void) 
{
    EIFR = _BV(INTF0); // clear any possibly pending
    EIMSK |= _BV(INT0); // enable INT0
}

inline static void hal_uart_init_receive_timeout(uint8_t timeout)
{
    timeout &= 0x1F; // only 1...31, else this is a bit much
    uart_timeout = F_CPU/BAUDRATE * FREELINE_TIME;
    uart_timeout += (uint16_t)timeout * F_CPU/BAUDRATE/3; // add timeout as 1/3 bits
}

inline static void hal_uart_init_transmit_timeout(void)
{   // not necessary on AVR, we have TX done interrupt
}

inline static void hal_uart_start_transmit_timeout(void)
{   // not necessary on AVR, we have TX done interrupt
}

inline static void hal_uart_stop_transmit_timeout(void)
{   // not necessary on AVR, we have TX done interrupt
}

inline static void hal_uart_clear_transmit_timeout(void)
{   // not necessary on AVR, we have TX done interrupt
}

inline static void hal_uart_start_receive_timeout(void)
{
    // this trigger is starting 2 timeouts, in this order:
    // 1. RX_TIMEOUT, the overflow interrupt, a constant time from the trigger
    // 2. FREELINE_TIME, the match interrupt, varies per address in hal_uart_init_receive_timeout()

    uint16_t count = TCNT1;
    OCR1A = count + F_CPU/BAUDRATE * RX_TIMEOUT; // fixed part for back to back timeout
    OCR1B = count + uart_timeout; // address dependent part for idle timeout

    TIFR1 = _BV(OCF1A) | _BV(OCF1B); // clear any pending interrupts before enabling
    TIMSK1 |= (_BV(OCIE1A) | _BV(OCIE1B)); // finally, enable overflow and match interrupt
}

// disable the first timeout after hal_uart_start_receive_timeout()
inline static void hal_uart_clear_receive_timeout(void)
{
    TIMSK1 &= ~_BV(OCIE1A); // disable compare a
}

// disable the second timeout after hal_uart_start_receive_timeout()
inline static void hal_uart_clear_receive_timeout2(void)
{
    TIMSK1 &= ~_BV(OCIE1B); // disable compare b
}


inline static void hal_uart_init_hardware(void)
{
    UBRRL = (uint8_t)(F_CPU / (16*BAUDRATE) - 1); // 11
    UCSRB = _BV(RXCIE) | _BV(TXCIE) | _BV(RXEN) | _BV(TXEN); // enable RX+TX with interrupts
    UCSRC = _BV(URSEL) | _BV(USBS) | _BV(UCSZ1) | _BV(UCSZ0); // 8N2
}

inline static uint8_t hal_uart_has_errors(void)
{
    return UCSRA & 0x1C;
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

inline static void hal_uart_start_tx_irq(void)
{
    UCSRB |= _BV(UDRIE);
}

inline static void hal_uart_stop_tx_irq(void)
{
    UCSRB &= ~_BV(UDRIE);
}



#endif // #ifndef _UART_PLATFORM_H
