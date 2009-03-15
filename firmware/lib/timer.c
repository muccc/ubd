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

/*! \file timer.c
    \brief Common timer services.
*/

#define __FILENUM__ 7 // every file needs to have a unique (8bit) ID

#include <stdint.h>
#include "hal.h"
#include "timer.h"
#include "bus.h"
#include "busuart.h"

/*************** module state ***************/

static uint32_t timer_delay_ticks;
static void (*timer_tick_callback)(void);

/*************** public API functions ***************/

uint32_t timer_ticks; // public member

void timer_init(void (*callback)(void), uint8_t initvalue)
{
    timer_tick_callback = callback;
    hal_timer_init(initvalue);
}

uint16_t timer_performance_counter(void)
{
    return hal_timer_performance_counter();
}

// delayed message sending
void timer_msg(uint32_t ticks)
{
    uint8_t sreg;

    //UDR = 'C';
	sreg = SREG;
	cli(); // avoid access while writing the uint32_t
    timer_delay_ticks = ticks;
    SREG = sreg; // sei();
}

// heartbeat interrupt
TIMER_TICK_ISR
{
    PROFILE(PF_ISRSTART_TIMER);
    hal_timer_clear_irq();

	timer_ticks++; // the global counter
    uart_tick();
	// test if a delay message is due
	if (timer_delay_ticks && --timer_delay_ticks == 0)
	{
        //UDR = 'B';while(1);
		struct msg message;
		message.id = e_timer;
		message.data = 0; // unused
		if (msg_post(&message) != 0) // queue full?
		{
			timer_delay_ticks = 1; // try again @ next tick
		}
	}

    timer_tick_callback();

#ifdef CFG_ADC
    hal_start_adc(); // do that last thing before we hopefully sleep again, reduces noise
#endif
    PROFILE(PF_ISREND_TIMER);
}
