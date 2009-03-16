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
#include "bus.h"
#include "busuart.h"
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

    uint8_t tx_done;

    uint8_t tx_wait;
} uart;

enum uart_error{
    UART_NULL,
    UART_OK,
    UART_TIMEOUT,
    UART_ERROR,
    UART_VERIFY
};
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
///    hal_uart_init_transmit_timeout();

	// init UART hardware
    hal_uart_init_hardware();
    uart.tx_wait = 0;
    uart.tx_done = 1;
}

void uart_randomize(uint8_t rand)
{
    hal_uart_init_receive_timeout(rand);
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
			tx_end(UART_ERROR); // tx error
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
				tx_end(UART_VERIFY); // tx verify error
			}
			else if (uart.tx_echo == uart.tx_size)  // done?
			{
				tx_end(UART_OK); // proper end of transmission
			}
			ASSERT(uart.tx_echo <= uart.tx_size);
			
			return; // exit, don't do normal RX processing
		}
		else
		{	// receiving
			bus_rcv_byte(data); // pass to input state machine
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
    ///    hal_uart_start_transmit_timeout();
    PROFILE(PF_ISREND_TX);
}

// transmit done, this comes later than the RX echo, can use this for timeout
UART_TX_DONE_ISR // all is sent
{
    PROFILE(PF_ISRSTART_TX_DONE);

    hal_uart_rs485_disable();
    if (uart.tx_active)
    {
    	tx_end(UART_TIMEOUT); // tx timeout error
    }

    PROFILE(PF_ISREND_TX_DONE);
}


// no more bytes following back to back on the line
UART_RX_TIMEOUT_ISR
{
    PROFILE(PF_ISRSTART_RX_TIMEOUT);
    hal_uart_clear_receive_timeout(); // single shot, disable this interrupt

	// discard the undecoded buffer content
	bus_timeout();

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
    PORTD ^= (1<<PD6);
    if(uart.tx_wait){
        hal_uart_rx_edge_disable(); // disable while sending, avoids excessive interrupts
	    hal_uart_rs485_enable(); // enable the RS485 driver
	//hal_delay_us(0.5 * 1000000.0/BAUDRATE); // wait for 0.5 bits time, driver settling
        hal_uart_start_tx_irq(); // enable tx interrupt, let the ISR send the first byte
        uart.tx_wait = 0;
    }

    PROFILE(PF_ISREND_RX_TIMEOUT2);
}


// send count bytes, blocking and verified
// returns 0 on success (no collision detected)

// FG
void uart_send(const uint8_t* buf, uint8_t count)
{
   
    uart.tx_buf = buf;
	uart.tx_size = count;
	uart.tx_send = 0;
	uart.tx_echo = 0;
	uart.tx_result = UART_NULL; // will be changed by ISR
    uart.tx_done = 0;
    if(uart.rx_notidle == 0){
         hal_uart_rx_edge_disable(); // disable while sending, avoids excessive interrupts
	    hal_uart_rs485_enable(); // enable the RS485 driver
	//hal_delay_us(0.5 * 1000000.0/BAUDRATE); // wait for 0.5 bits time, driver settling
        hal_uart_start_tx_irq(); // enable tx interrupt, let the ISR send the first byte
    }else{
        uart.tx_wait = 1;
    }
	
//        return uart.tx_result == 1 ? 0 : uart.tx_result; // 0 on success, else error code
}

void uart_tick(void)
{
    if(uart.tx_done)
        return;
    if (uart.tx_result){
        uart.tx_done=1;
        // Nasty workaround for an open bug:
        // It happens that the edge interrupt is not re-enabled or uart.tx_active is not cleared.
        // Apparently tx_end() wasn't called by ISR or did an incomplete job!!??
        // However, the transmission loop ended, with no timeout.
        // symptom fix, should never stay on
    	uart.tx_active = 0;
        hal_uart_rx_edge_enable(); // watch for falling RX again
    }
}


// used for waiting for "idle line"

// ISR, FG, reentrant
uint8_t uart_is_busy(void)
{
	return uart.rx_notidle;
}

uint8_t uart_txresult(void)
{
    return uart.tx_result;
}

void uart_txreset(void)
{
    uart.tx_result = UART_NULL;
}
