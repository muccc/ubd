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

/*! \file switch.c
    \brief Application helper part of input modules.
    
     There is a derivative of this file in "Src_IM/rc5.c". 
     Make sure to propagate relevant changes there, too!
*/

#define __FILENUM__ 12 // every file needs to have a unique (8bit) ID

#include <stdint.h>
#include "hal.h"
#include "uart.h"
#include "msg.h"
#include "timer.h"
#include "phc.h"
#include "switch.h"

#define HIST 3 // debounce history
#define BUFSIZE (CFG_INPUT*4) // event buffer size, 32 or 64 depending on makefile

#define BUF_MASK (BUFSIZE - 1)
#if (BUFSIZE & BUF_MASK) // must be power of 2
	#error BUFSIZE is not a power of 2
#endif

/*************** module state ***************/

static struct _global_switch_context
{   // codesize saver: place the most frequented on top, cluster them like locally used
    switch_t state; // stable state of the switches
    switch_t debounce[HIST];
    uint16_t on_since[NUM_INPUTS]; // measure keypress time

    // state of the event queue
    // ToDo: check if all these index memories are necessary
    uint8_t eventqueue[BUFSIZE]; 
    uint8_t queue_head; // write position, maintained by ISR (timer tick)
    uint8_t queue_post; // posted up to here, to detect new entries, maintained by ISR (timer tick)
    uint8_t queue_sent; // handed out up to here, maintained by main loop
    uint8_t queue_tail; // acknowledged up to here, maintained by main loop
    uint8_t wait_for_ack; // block further messages, wait for roundtrip ack

    // configuration data
    switch_t event_enable[INPUT_OFF+1]; // which input shall send what, index is function offset
} switches;

/*************** private internal functions ***************/

// store a single event
static void post_event(uint8_t channel, uint8_t id)
{
    switch_t mask = (switch_t)1 << channel;
    if (switches.event_enable[id] & mask) // enabled by configuration?
    {
        uint8_t tmphead;
        uint8_t entry = channel << 4 | id;

	    tmphead = (switches.queue_head + 1) & BUF_MASK;
        if (tmphead != switches.queue_tail)
	    {
		    switches.queue_head = tmphead; // store new index
		    switches.eventqueue[tmphead] = entry; // store new entry in queue
	    }
        else
	    {	// ERROR! Message queue full
		    ASSERT(0); // log this condition
	    }

    }
}

// send a message if the event queue is "dirty" (has new entries)
static void flush_events(void)
{
	struct msg message;
    
    if (switches.queue_post == switches.queue_head)
        return; // nothing to do
    
	message.id = e_event;
	message.data = switches.queue_head;
	if (msg_post(&message) == 0) // success in posting
    {
        switches.queue_post = switches.queue_head;
        switches.wait_for_ack = 1;
    }
    // else it will be retried next tick
}

/*************** public API functions ***************/


void switch_init(void)
{
    // it turnes out all we need here is to rely on the BSS zero init
}

// poll the switches, with individual bit-wise debouncing (in EM the ISR takes ~0.2ms)
void switch_tick(void)
{
	uint8_t i;
	switch_t newstate = hal_get_switch(); // 8 resp. 16 bit
	switch_t stable; // bit flag if the queue contains all the same values per bit position
    switch_t oldstate = switches.state;
	
	// debouncing
	stable = ~(switches.debounce[0] ^ newstate); // begin check with the oldest
	for (i=0; i<HIST-1; i++) // check and move the values
	{	// set the same into the debounce queue
		switches.debounce[i] = switches.debounce[i+1];
		stable &= ~(switches.debounce[i] ^ newstate);
	}
	switches.debounce[HIST-1] = newstate; // topmost becomes the new value

    // copy the bits which are stable 
    newstate = (newstate & stable) | (oldstate & ~stable);
    switches.state = newstate; // copy into global

	// check through all the switches
	for (i=0; i<NUM_INPUTS; i++)
	{
		switch_t mask = (switch_t)1 << i;

        if (newstate & mask) // switch is held pressed
		{
			if (switches.on_since[i] < HZ*2) // saturated counting avoids wrap
            {
                switches.on_since[i]++; // increment the time
			    if (switches.on_since[i] == HZ*1)
			    {
				    post_event(i, INPUT_ON_GREATER_1);
			    }
			    else if (switches.on_since[i] == HZ*2)
			    {
				    post_event(i, INPUT_ON_GREATER_2);
			    }
            }
		} // if held
		
		if ((newstate & mask) != (oldstate & mask)) // different
		{
            if (newstate & mask)
			{ // switched on
				post_event(i, INPUT_ON_GREATER_0);
				switches.on_since[i] = 0; // start anew
			}
			else
			{ // switched off
				post_event(i, INPUT_OFF);
				if (switches.on_since[i] > HZ*1)
                {
    				post_event(i, INPUT_OFF_GREATER_1);
                }
                else
                {
    				post_event(i, INPUT_OFF_LESS_1);
                }
			}
		} // if different
	} // for i

    if (!switches.wait_for_ack // no pending msg?
	 && !uart_is_busy()) // no other traffic?
	{
        flush_events(); // if new entries, this will send a message to main loop
    } // if (!switches.wait_for_ack)

}


// set the enable bits, the filter configuration
void switch_set_enable(switch_t* p_enables)
{
    uint8_t i;
    for (i=INPUT_ON_GREATER_0; i<=INPUT_OFF; i++)
    {
        switches.event_enable[i] = p_enables[i];
    }
}

switch_t switch_getcurrent(void)
{
    return switches.state;
}

void switch_setcurrent(switch_t lines)
{
    switches.state = lines; // the tick ISR will signal changes, if any
}

// copy the queue content into a caller-supplied buffer, returns size
uint8_t switch_getevents(uint8_t* dest, uint8_t bufsize, uint8_t pos)
{
    uint8_t count = 0;
    uint8_t i = switches.queue_tail;
    while (i != pos && count < bufsize)
    {
        i = (i + 1) & BUF_MASK;
        *dest++ = switches.eventqueue[i];
        count++;
    }
    switches.queue_sent = i; // store how far we gave
    return count;
}

// roundtrip ack, we now have seen the ack from the control unit
void switch_ack(void) 
{
    switches.queue_tail = switches.queue_sent; // advance the tail
    switches.queue_post = switches.queue_tail; // re-post if only a part was taken
	switches.wait_for_ack = 0; // allow next message
}

// roundtrip negative ack, we can't confirm the control unit got it
void switch_nack(void) 
{
    switches.queue_post = switches.queue_tail; // the ISR will re-post
    switches.wait_for_ack = 0; // allow next message
}
