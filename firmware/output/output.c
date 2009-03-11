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

/*! \file output.c
    \brief Application layer of the AM module
    
     There is a derivative of this file in "Src_JRM/jrm.c". 
     Make sure to propagate relevant changes there, too!
*/

#define __FILENUM__ 14 // every file needs to have a unique (8bit) ID

#include <stdint.h>
#include "msg.h"
#include "timer.h"
#include "hal.h"
#include "phc.h"
#include "uart.h"
#include "random.h"
#include "output.h"
#include "softuart.h"

#define BOOST_TICKS (HZ/25) // how long 100% power to relays switching on
#define PWM_ADDR 0x100 // EEPROM location of PWM fraction for relay holding

#define BUFSIZE 16 // event buffer size (must be power of 2)

#define BUF_MASK (BUFSIZE - 1)
#if (BUFSIZE & BUF_MASK)
	#error BUFSIZE is not a power of 2
#endif


// actions after time expiration
enum e_action
{
	act_nop, // do nothing
    act_on,
    act_off,
    act_toggle,
	act_last	
};

// PHC receiver state
enum e_outputstate
{
	st_idle, // not expecting any ack or timeout
	st_ack, // expecting plain event ack
	st_cfg, // expecting configuration
	st_last
};


/*************** Output module state ***************/

static struct _global_am_context
{   // codesize saver: place the most frequented on top, cluster them like locally used

    uint8_t lines; // state of the physical output lines
    uint8_t feedback_enable[OUTPUT_FB_OFF+1]; // which output shall send what feedback, index is function
    uint8_t new_enable[OUTPUT_FB_OFF+1]; // same, but working copy during ISR
    uint8_t por; // POR state as sent by ISR
    uint32_t time[8]; // time in ticks per channel
    enum e_action action[8]; // action on timer expiration, per channel
    uint8_t unlock; // bit flags for unlock upon timer expiration
    uint8_t lock; // lock bits
    uint16_t new_time; // seconds as received in ISR
    uint8_t toggle; // toggle bit for self-initiated packets
    uint8_t modul_addr; // class and module address byte
    uint8_t is_bootcmd; // special flag to test for bootloader mode cmd, too
    uint8_t len; // store the command length to aid final validation
    // uint8_t disabled; // instructed to shut up
    int8_t boost_ticks; // high-power time for relays, during switch-on
    uint8_t pwm; // nominal PWM ratio in 1/256 units
    uint8_t retry_wait; // packet retry time
    enum e_outputstate state;

    // state for interrupt context packet parsing
    uint8_t addr;
    uint8_t rcv_toggle; // toggle bit of received packet
    uint8_t listening; // flag if we're interested
    uint8_t cmd; // channel/command byte

    // state of the event queue
    uint8_t eventqueue[BUFSIZE]; 
    uint8_t queue_head; // write position, maintained by ISR (timer tick)
    uint8_t queue_tail; // acknowledged up to here, maintained by main loop
    uint8_t queue_wait4ack; // notify message is sent, waiting for main loop
} output;


/*************** private internal functions ***************/

// send an internal message, from ISR to main loop
static void send_msg_reply(uint8_t id)
{
	struct msg message;
	message.id = id;
	message.data = output.rcv_toggle;
	if (msg_post(&message) != 0) // overflow, problem
	{
		ASSERT(0);
	}
}

// add an entry to the event queue
static void post_event(uint8_t event, uint8_t channel)
{
    uint8_t newhead;

    newhead = (output.queue_head + 1) & BUF_MASK;
    if (newhead != output.queue_tail) // no overflow
    {
        event |= channel << 5; // add the channel #, by chance this doesn't disturb OUTPUT_FB_TIMER
        output.eventqueue[newhead] = event;
        output.queue_head = newhead;
    }
}

// send a message on non-empty event queue and idle
static void notify_event(void)
{
    if (!output.queue_wait4ack // not already pending message
        && output.queue_head != output.queue_tail // queue not empty
        && !uart_is_busy()) // not busy receiving other stuff
    {
        struct msg message;

	    message.id = e_event;
	    message.data = 0; // ToDo: useful payload?
	    if (msg_post(&message) == 0) 
        {   // success: place in the event queue
            output.queue_wait4ack = 1;
        }
        else
	    {   // overflow, problem
		    ASSERT(0);
	    }
        
    }

}

// set new output value
static void set_output(uint8_t new_lines)
{
    if (new_lines == output.lines)
    {
        return; // nothing to do
    }

    if (new_lines & ~output.lines) // new set bit(s)?
    {
        hal_set_pwm(255); // full power while activating relays
        output.boost_ticks = BOOST_TICKS;
    }
    else if (new_lines == 0)
    {
        hal_set_pwm(0); // DC on to reduce EMI, no pulses
        output.boost_ticks = -1; // no PWM
    }
    
    output.lines = new_lines; // finally, set the new levels
    hal_set_output(output.lines);
}

/*************** public API functions ***************/

// assign a bus address
void output_init(uint8_t addr)
{
	output.modul_addr = addr; // class and address
	
	//output.disabled = 0;
	output.state = st_cfg;
    output.boost_ticks = -1;
    output.retry_wait = ACK_TO;

    output.pwm = hal_eeprom_read_byte((void*)PWM_ADDR);
    hal_set_pwm(0); // DC power to relays
	
	//timer_msg(HZ*3); // send the boot message in 3 seconds

    // send the boot message almost immediately, 3 sec wait was done by bootloader
    timer_msg(2); 
}

// the main loop of the PHC output application
void output_mainloop(void)
{
	uint8_t response[2+2+2]; // worst-case size with addr+length, payload, CRC
	uint8_t event; // our to-be-generated switch message
	
	while (1)
	{
		struct msg message;

		event = 0; // we check for change later
		message = msg_get();
        softuart_puts("msg.id=");
        softuart_putchar( message.id+'0');
        softuart_puts("\r\n");
		switch(message.id)
		{
		case e_send_state: // send output line state as ack
		case e_send_ping: // send ping response
            response[2] = (message.id == e_send_state) ? 0x00 : 0x01;
			response[3] = output.lines;
            // no error handling necessary, if no success in sending our reply, the sender will re-post
			phc_send(output.modul_addr, response, 2, message.data);
			break;
		
        case e_event:
            //if (!output.disabled)
            {
                uint8_t newtail = (output.queue_tail + 1) & BUF_MASK;
                ASSERT(output.queue_head != output.queue_tail); // not empty
                response[2] = output.eventqueue[newtail];
			    response[3] = output.lines;
			    if (!uart_is_busy() && 
                    phc_send(output.modul_addr, response, 2, output.toggle) == 0)
                {   // successsful sent
			        output.state = st_ack;
	                timer_msg(output.retry_wait); // ack timeout with backoff period
                }
                else
                {
                    output.queue_wait4ack = 0; // event will be re-posted in tick ISR
                }
            }
            break;

        case e_timer:
            //UDR = 't';while(1);
            output.retry_wait += 1 + (rand() & 0x03); // back off, increase timeout (wraps on purpose at 255)

            if (output.retry_wait < ACK_TO)
            {   // most of all, must not become zero, that would cancel the timer
                output.retry_wait += ACK_TO;
            }
        
			if (output.state == st_cfg)
			{
                if (!uart_is_busy()) // about to send unsolicited: only with idle line
                {
                    response[2] = 0xFF;
                    phc_send(output.modul_addr, response, 1, output.toggle); // boot message
                }

                timer_msg(output.retry_wait); // send ourself a message in a few ticks, response timeout
			}
	        else if (output.state == st_ack) // waiting for ack
	        {
                output.queue_wait4ack = 0; // allow next posting, within timer
                output.state = st_idle;
	        }
			break;

        default: // unhandled message
			ASSERT(0);
		} // case
	} // while(1)
}


// interrupt context functions for incoming packet processing

// packet start
void output_cmd_start(uint8_t address, uint8_t toggle, uint8_t len)
{
	output.addr = address;
	output.rcv_toggle = toggle;
	output.listening = (address == output.modul_addr || (address & 0xE0) == 0xE0); // listen for POR, too
    output.is_bootcmd = (len == 3); // to be refined during payload
    output.len = len; // store length for later validation
}

void output_payload(uint8_t pos, uint8_t byte)
{
	if (!output.listening)
	{
		return; // short cut
	}
	
	if (pos == 0) // channel/command byte
	{
		output.cmd = byte;
        if (output.cmd != 0x01) // ping, could become magic boot ping
        {
            output.is_bootcmd = 0; // no: falsify
        }
		
		// prepare non-simple commands
        output.new_time = 0; // clean, so it'll be 0 if no time given
		if (output.cmd == 0xFE) // configuration
		{
			output.new_enable[OUTPUT_FB_ON] = output.new_enable[OUTPUT_FB_OFF] = 0;
		}
		return;
	}

    if (output.is_bootcmd) // candidate for magic boot ping
    {
    	if ((pos == 1 && byte != 'B')
         || (pos == 2 && byte != 'L'))
        {
            output.is_bootcmd = 0; // mismatch: falsify
        }
    }

    // preliminary process the non-simple commands, everything with payload
	// the CRC may prove wrong later on, so don't do anything permanent	

	if (output.cmd == 0xFE) // configuration
	{
		if (pos == 1)
        {
            output.por = byte; // remember POR value
        }
        else if (pos > 2) // event configuration part
		{
			uint8_t channel = byte >> 5;
			uint8_t function = byte & 0x1F;
			if (function <= OUTPUT_FB_OFF) // 2...3 legal range
			{
				output.new_enable[function] |= (uint8_t)1 << channel;
			}
		}
	}
    else if (pos == 1) // larger than single byte commands carry time
    {
        output.new_time = byte;
    }
    else if (pos == 2)
    {
        output.new_time |= (uint16_t)byte << 8;
    }
}

// packet end
void output_cmd_end(uint8_t valid, uint8_t retry)
{
	if (!output.listening)
	{
		return; // short cut
	}

	if (!valid)
	{	// we don't care, nothing to be un-done
		return; 
	}

    if ((output.addr & 0xE0) == 0xE0) // broadcast command
    {
        // ToDo: validate the global commands
        switch (output.cmd)
        {
        case 0xFF: // POR, ToDo: meaning of payload byte, hard/soft POR?
            hal_reboot();
		    break; // won't continue here, but with a break the code gets smaller

        case 0xFE: // unknown???
            // ToDo
            break;
        }
    }
    else // command to our specific address
    {
        if (output.is_bootcmd)
        {
            // execute bootloader
            hal_reboot();
            // won't continue here
        }

	    switch (output.cmd) // first look at the whole command byte
	    {
	    case 0x00: // ack
		    if (output.state == st_ack)
		    {
			    output.state = st_idle;
		        output.toggle = !output.toggle; // late toggle, on roundtrip success
                output.retry_wait = ACK_TO; // reset the timeout
			    timer_msg(0); // cancel timer

                // Fixme: better do this in main loop, via msg?
                output.queue_tail = (output.queue_tail + 1) & BUF_MASK; // remove tail entry
                output.queue_wait4ack = 0; // allow next posting
		    }
		    break;

	    case 0x01: // ping
		    send_msg_reply(e_send_ping);
		    break;
	    
	    case 0xFE: // configuration
		    {
			    set_output(output.por); // set the POR levels

                // copy the new config
			    output.feedback_enable[OUTPUT_FB_ON] = output.new_enable[OUTPUT_FB_ON];
			    output.feedback_enable[OUTPUT_FB_OFF] = output.new_enable[OUTPUT_FB_OFF];

			    if (output.state == st_cfg)
			    {
				    output.state = st_idle;
				    timer_msg(0); // cancel timer
			    }
		    }
		    break;
		    
	    case 0xFF: // reset
//            if (output.len == 1) // length 2 could be a boot msg. of a module with same adr.!
            {
                hal_reboot(); // warm reset, goes through bootloader if avail.
            }
		    break; // won't continue here, but with a break the code gets smaller
		    
	    default: // then check for channel/function pair
		    {
			    uint8_t channel = output.cmd >> 5;
			    uint8_t function = output.cmd & 0x1F;
                uint8_t mask = (uint8_t)1 << channel;
                uint32_t time = (uint32_t)output.new_time * HZ; // 0 if not set
                uint8_t cleartime = 0; // set for all ops which override a pending timed one

                // copy current state
                uint8_t new_lines = output.lines;
                uint8_t new_lock = output.lock;

                if (retry // repeated command: our reply wasn't received, action already happened last time
                    || ((output.lock & mask) && function < OUTPUT_UNLOCK)) // when locked, accept no switch cmd
                {
                    function = 0; // now do nothing
                    time = 0;
                }

			    // ToDo: move into a function?
                switch (function)
			    {
                case OUTPUT_LOCK_ON: // Einschalten verriegelt
                    new_lock |= mask;
                    // fall through
                case OUTPUT_ON: // Einschalten
				    new_lines |= mask;
                    cleartime = 1;
				    break;	

                case OUTPUT_LOCK_OFF: // Ausschalten verriegelt
                    new_lock &= ~mask;
                    // fall through
                case OUTPUT_OFF: // Ausschalten
				    new_lines &= ~mask;
                    cleartime = 1;
				    break;	

                case OUTPUT_TOGGLE: // Umschalten
				    new_lines ^= mask;
                    cleartime = 1;
                    break;

                case OUTPUT_UNLOCK: // Entriegeln
                    new_lock &= ~mask;
                    break;

                case OUTPUT_DELAY_ON: // Einschaltverzögerung
                    output.action[channel] = act_on;
                    break;

                case OUTPUT_DELAY_OFF: // Ausschaltverzögerung
                    output.action[channel] = act_off;
                    break;

                case OUTPUT_TIMED_ON: // Einschalten mit Zeitglied
				    new_lines |= mask;
                    output.action[channel] = act_off;
                    break;

                case OUTPUT_TIMED_OFF: // Ausschalten mit Zeitglied
				    new_lines &= ~mask;
                    output.action[channel] = act_on;
                    break;

                case OUTPUT_DELAY_TOGGLE: // Verzögert umschalten, zeitverriegelt
                    new_lock |= mask;
                    output.unlock |= mask;
                    output.action[channel] = act_toggle;
                    break;

                case OUTPUT_TIMED_TOGGLE: // Umschalten mit Zeitglied, zeitverriegelt
                    new_lock |= mask;
                    output.unlock |= mask;
				    new_lines ^= mask;
                    output.action[channel] = act_toggle;
                    break;

                case OUTPUT_LOCK: // Fest verriegeln
                    new_lock |= mask;
                    cleartime = 1; // ToDo: really cancel pending cmd?
                    break;

                case OUTPUT_TIMED_LOCK: // Verriegeln für laufende Zeit
                    // has a side-effect of permanently locking if no time running, like original
                    new_lock |= mask;
                    output.unlock |= mask;
                    break;

                case OUTPUT_TIME_ADD: // Zeitaddition auf laufende Zeit
                    time += output.time[channel]; // will be set below
                    break;

                case OUTPUT_TIME_SET: // Zeit neu setzen
                    // done below
                    break;

                case OUTPUT_TIME_CANCEL: // Zeitabbruch
                    cleartime = 1;
                    output.action[channel] = act_nop;
                    break;

                default:
                    break;
			    }

                if (time) // present and not overridden above?
                {
                    output.time[channel] = time;
                }

                if (cleartime) // the new command would cancel a pending one
                {
                    output.time[channel] = 0;
                    output.unlock &= ~mask;
                }
        
                // change only the non-locked
                new_lines = (output.lines & output.lock) // take the old if locked (ToDo: not necessary if we block locked like above)
                          | (new_lines & ~output.lock); // take the new if unlocked

                output.lock = new_lock; // finally, set the new locks
       		    send_msg_reply(e_send_state); // default command answer

                // send feedback, if enabled and changed
                if (new_lines > output.lines && (output.feedback_enable[OUTPUT_FB_ON] & mask))
                {   // switched on
                    post_event(OUTPUT_FB_ON, channel);
                }
                else if (new_lines < output.lines && (output.feedback_enable[OUTPUT_FB_OFF] & mask))
                {   // switched off
                    post_event(OUTPUT_FB_OFF, channel);
                }

                set_output(new_lines); // also sets output.lines
            } // default	
        } // switch (output.cmd)
    } // if no broadcast cmd
		
	// Still waiting for an ack: treat any new command as negative ack,
	//  the real one may have gotten lost.
	if (output.state == st_ack)
	{
        output.queue_wait4ack = 0; // allow next posting, within timer

        output.state = st_idle;
		timer_msg(0); // cancel timer
	}
}

// also an ISR, so it won't compete against output_cmd_end()
void output_tick(void)
{
    uint8_t i;
    uint8_t mask = 0x01;
    uint8_t new_lines = output.lines;
    
#ifdef CFG_ADC // constant monitoring
    if (output.boost_ticks > 0)
        output.boost_ticks--; // only count down to 0

    if (output.boost_ticks == 0) // expired?
    {
        uint32_t pwm = output.pwm; // reduced power with PWM
        // tune the PWM value by actually measured supply voltage
        if (pwm < 255)
        {   // no full power intended
            pwm = (pwm * 24000L) / hal_read_voltage();
            if (pwm > 255) // cap if it became too large, >100%
            {
                pwm = 255;
            }
       }
       hal_set_pwm(pwm);
    }
#else // only interested in edge
    if (output.boost_ticks > 0 && --output.boost_ticks == 0)
    {
       hal_set_pwm(output.pwm);
    }
#endif

    for (i=0; i<8; i++)
    {
        if (output.time[i] && --output.time[i] == 0)
        {   // was running, now expired
            switch (output.action[i])
            {
            case act_on:
                new_lines |= mask;
                break;

            case act_off:
                new_lines &= ~mask;
                break;

            case act_toggle:
                new_lines ^= mask;
                break;
            
            default:
                ASSERT(output.action[i] == act_nop); // else illegal
                break;
            }
            output.action[i] = act_nop;
            
            if (output.unlock & mask) // pending unlock
            {
                output.lock &= ~mask;
                output.unlock &= ~mask;
            }

            // send feedback, if enabled and changed
            if (new_lines > output.lines && (output.feedback_enable[OUTPUT_FB_ON] & mask))
            {   // switched on
                post_event(OUTPUT_FB_ON, i);
            }
            else if (new_lines < output.lines && (output.feedback_enable[OUTPUT_FB_OFF] & mask))
            {   // switched off
                post_event(OUTPUT_FB_OFF, i);
            }
            else if (output.lines != new_lines)
            {   // if changed but no feedback enabled: send expiration status
                post_event(OUTPUT_FB_TIMER, i);
            }
        
            set_output(new_lines); // also sets output.lines

            break; // exit the loop, do one at a time (avoid problem of multiple expirations)
                   //  check, might be possible with event queue now?
        }

        mask <<= 1;
    } // for i

    notify_event();
}
