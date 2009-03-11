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

/*! \file uart.c
    \brief Serial port driver, specialized for PHC.
    
    All the action happens within interrupts.
*/

#define __FILENUM__ 8 // every file needs to have a unique (8bit) ID

#include <stdint.h>
#include "hal.h"
#include "msg.h"
#include "phc.h"
#include "uart.h"
#include "timer.h"

// pick the right "mini driver" to resolve hardware dependencies
#include "uart_platform.h"

#define TIMEOUT (HZ/10) // time to transmit a worst-case packet

// static variables
static struct _global_uart_context
{   // codesize saver: place the most frequented on top, cluster them like locally used

    // ISR rw, FG read
    volatile uint8_t rx_active; // flag we're receiving, not timed out yet
    volatile uint8_t rx_notidle; // flag that the line is not considered free

    // ISR rw, FG read -> check
    volatile uint8_t tx_active; // flag we're sending

    // ISR write, FG rw -> check?
    volatile uint8_t tx_result; // transmit result as detected by ISR

    // ISR rw, FG write
    uint8_t tx_echo; // readback (echo) position

    // ISR read, FG write
    const uint8_t* tx_buf; // valid while transmitting

    // ISR rw, FG write
    uint8_t tx_send; // send position

    // ISR read, FG write
    uint8_t tx_size; // # of bytes in above buffer to send
} uart;


/* private functions */

// ISR, one FG exception
void tx_end(uint8_t errcode)
{
	// don't disable the RS485 driver here, in normal case the 2nd stop bit
    //  is still being transferred, will disable in TX done interrupt
    uart.tx_result = errcode;
	uart.tx_active = 0;
    hal_uart_rx_edge_enable(); // watch for falling RX again
}


/* public API functions */

// FG
void uart_init(uint8_t timeout)
{
	// timer for receive timeout
    hal_uart_init_receive_timeout(timeout);
			
	// timer for transmit timeout, if no echo received
    hal_uart_init_transmit_timeout();

	// init UART hardware
    hal_uart_init_hardware();
}

// RX activity ISR
#ifdef UART_RX_EDGE_ISR
UART_RX_EDGE_ISR
{
    PROFILE(PF_ISRSTART_RX_EDGE);
    //ASSERT(!uart.tx_active); // ToDo: this assertion is triggering!
    if (!uart.tx_active)
	{
		// spawn receive timeout
        hal_uart_start_receive_timeout(); // in case no RX interrupt follows (spike)
		uart.rx_active = 1;
		uart.rx_notidle = 1;
	}
    hal_uart_rx_edge_disable(); // disable ourself, avoid excessive interrupt load during receive
    PROFILE(PF_ISREND_RX_EDGE);
}
#endif // #ifdef UART_RX_EDGE_ISR

// receive interrupt handler
UART_RX_ISR
{
    PROFILE(PF_ISRSTART_RX);
    if (!uart.tx_active)
	{
		// (re)spawn receive timeout
        hal_uart_start_receive_timeout();
		uart.rx_active = 1; // strictly speaking, only needed if no edge detection, but better be safe
        uart.rx_notidle = 1;
	}

	if (hal_uart_has_errors()) // framing, overrun, parity error?
	{
		hal_uart_clear_errors(); // clear error flags (and interrupt)
		
		if (uart.tx_active) // we're transmitting, waiting for echo
		{
			tx_end(2); // tx error
		}
		// else ignore error
	}

	if (hal_uart_has_data()) // received data?
	{
		uint8_t data;

		data = hal_uart_read_data(); // read the received data, clear interrupt

		if (uart.tx_active) // we're transmitting, waiting for echo
		{
			uint8_t verify;
			
			verify = uart.tx_buf[uart.tx_echo++];
			
			if (data != verify) // must match previously sent
			{
				tx_end(3); // tx verify error
			}
			else if (uart.tx_echo == uart.tx_size)  // done?
			{
				tx_end(1); // proper end of transmission
			}
			ASSERT(uart.tx_echo <= uart.tx_size);
			
			return; // exit, don't do normal RX processing
		}
		else
		{	// receiving
			phc_rcv_byte(data); // pass to input state machine
		}
	}
    PROFILE(PF_ISREND_RX);
}

// transmit interrupt handler
UART_TX_ISR
{
    PROFILE(PF_ISRSTART_TX);
	uart.tx_active = 1;
	ASSERT(uart.tx_send <= uart.tx_size);
	if (uart.tx_send < uart.tx_size) // check if there is data left
	{
		hal_uart_send_data(uart.tx_buf[uart.tx_send++]); // start/continue transmition
	}
	else
	{
        hal_uart_stop_tx_irq(); // disable tx interrupt
	}   

	// (re)start the tx timeout
    hal_uart_start_transmit_timeout();
    PROFILE(PF_ISREND_TX);
}

// transmit done, this comes later than the RX echo, can use this for timeout
#ifdef UART_TX_DONE_ISR
UART_TX_DONE_ISR // all is sent
{
    PROFILE(PF_ISRSTART_TX_DONE);

    hal_uart_rs485_disable();
    if (uart.tx_active)
    {
    	tx_end(4); // tx timeout error
    }

    PROFILE(PF_ISREND_TX_DONE);
}
// tx timeout, the way to go for less sophisticated UARTs
#elif defined(UART_TX_TIMEOUT_ISR)
UART_TX_TIMEOUT_ISR
{
    PROFILE(PF_ISRSTART_TX_TIMEOUT);

    hal_uart_clear_transmit_timeout(); // clear interrupt
    hal_uart_stop_transmit_timeout();
    hal_uart_rs485_disable();
    
    if (uart.tx_active) // missed the echo or sending too slow
    {
    	tx_end(4); // tx timeout error
    }
    
    PROFILE(PF_ISREND_TX_TIMEOUT);
}
#else
#error "no end of transmission implementation"
#endif


// no more bytes following back to back on the line
UART_RX_TIMEOUT_ISR
{
    PROFILE(PF_ISRSTART_RX_TIMEOUT);
    hal_uart_clear_receive_timeout(); // single shot, disable this interrupt

	// discard the undecoded buffer content
	phc_timeout();

	uart.rx_active = 0; // signal to the foreground
    hal_uart_rx_edge_enable(); // watch for falling RX edge again
    PROFILE(PF_ISREND_RX_TIMEOUT);
}

// the line is idle long enough to allow unsolicited sending
UART_RX_TIMEOUT2_ISR
{
    PROFILE(PF_ISRSTART_RX_TIMEOUT2);
    hal_uart_clear_receive_timeout2(); // single shot, disable this interrupt
    uart.rx_notidle = 0; // signal to the foreground
    PROFILE(PF_ISREND_RX_TIMEOUT2);
}


// send count bytes, blocking and verified
// returns 0 on success (no collision detected)

// FG
uint8_t uart_send(const uint8_t* buf, uint8_t count)
{
	uint32_t timeout;
	
	ASSERT(count > 0);

	// wait for the receive timeout, in case we're just turning from listening to sending
    // (using the timeout for this 2nd purpose has the slight disadvantage that we 
    // could actually reply earlier, this way we always respond after > 1 byte idle)
    for (;;) 
    {
        uint8_t sreg;

		sreg = SREG;
		cli(); // make the check below atomic, else in rare race condition we may oversleep
		if (!uart.rx_active) // OK, exit
		{
			SREG = sreg; // sei();
			break;
		}
        PROFILE(PF_SLEEP);
        hal_sleep_enable();
        SREG = sreg; // sei();
	    hal_sleep_cpu(); // trick: the instruction behind sei gets executed atomic, too
	    hal_sleep_disable();
    }

    // not needed any more, we waited for the timeout
    //hal_uart_clear_receive_timeout(); // important, the ISR would enable rx_edge

    hal_uart_rx_edge_disable(); // disable while sending, avoids excessive interrupts
	hal_uart_rs485_enable(); // enable the RS485 driver
	hal_delay_us(0.5 * 1000000.0/BAUDRATE); // wait for 0.5 bits time, driver settling

	//uart.rx_active = 0; // in case this was still set
	uart.rx_notidle = 0; // in case this was still set

    uart.tx_buf = buf;
	uart.tx_size = count;
	uart.tx_send = 0;
	uart.tx_echo = 0;
	uart.tx_result = 0; // will be changed by ISR

	timeout = timer_ticks; // debug
    ASSERT(!uart.tx_active);
    hal_uart_start_tx_irq(); // enable tx interrupt, let the ISR send the first byte
	
	for (;;)
	{
        uint8_t sreg;

		sreg = SREG;
        cli(); // make the check below atomic, else in rare race condition we may oversleep
		if (uart.tx_result || (timer_ticks - timeout) >= TIMEOUT)
		{
			SREG = sreg; // sei();
			break;
		}
        PROFILE(PF_SLEEP);
        hal_sleep_enable();
        SREG = sreg; // sei();
	    hal_sleep_cpu(); // trick: the instruction behind sei gets executed atomic, too
	    hal_sleep_disable();
    }

	ASSERT(uart.tx_result != 0); // must not time out
    if ((timer_ticks - timeout) >= TIMEOUT)
    {
        hal_uart_rs485_disable(); // should never happen, but better be safe
        ASSERT(0);
    }
    //ASSERT(GICR & _BV(INT0)); // must be enabled again  ToDo: triggers sometimes, still happens with rev. 91
    ASSERT(!uart.tx_active); // must be cleared         ToDo: triggers rarely

    // Nasty workaround for an open bug:
    // It happens that the edge interrupt is not re-enabled or uart.tx_active is not cleared.
    // Apparently tx_end() wasn't called by ISR or did an incomplete job!!??
    // However, the transmission loop ended, with no timeout.
    tx_end(0); // symptom fix, should never stay on

    return uart.tx_result == 1 ? 0 : uart.tx_result; // 0 on success, else error code
}

// used for waiting for "idle line"

// ISR, FG, reentrant
uint8_t uart_is_busy(void)
{
	return uart.rx_notidle;
}
