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

/*! \file msg.c
    \brief Interrupt-safe message queue implementation
    
    A mean and lean implementation of a message queue.
    Main thread or interrupts can post messages,
    the foreground loop waits and picks a message.
    The controller is sent to a low power sleep mode while waiting.
*/

#define __FILENUM__ 4 // every file needs to have a unique (8bit) ID

#include <stdint.h>
#include "hal.h"
#include "msg.h"
#include "uart.h"

#define MSG_QUEUESIZE 8

#define MSG_MASK (MSG_QUEUESIZE - 1)
#if (MSG_QUEUESIZE & MSG_MASK)
	#error MSG_QUEUESIZE is not a power of 2
#endif


static struct msg msg_queue[MSG_QUEUESIZE];
static volatile uint8_t msg_head;
static volatile uint8_t msg_tail;


// post a message, also allowed from interrupt context
uint8_t msg_post(struct msg* message)
{
	uint8_t tmphead;

	tmphead = (msg_head + 1) & MSG_MASK;

    if (tmphead != msg_tail)
	{
		msg_head = tmphead; // store new index
		msg_queue[tmphead] = *message; // store received message in queue
		return 0;
	}
	else
	{	// ERROR! Message queue full
		// set debug bit for indication
		return 1;
	}
}

// wait for a message
struct msg msg_get(void)
{
	uint8_t tmptail;

    for(;;) 
    {
        uint8_t sreg;

        hal_watchdog_reset(); // nice place to retrigger

		sreg = SREG;
        cli(); // make the check below atomic, else in rare race condition we may sleep despite new msg.
        if (msg_head != msg_tail)
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

    //pointless ASSERT((msg_head != msg_tail));

	tmptail = (msg_tail + 1) & MSG_MASK; // calculate buffer index
	
	msg_tail = tmptail; // store new index
	return msg_queue[tmptail]; // return message
}
