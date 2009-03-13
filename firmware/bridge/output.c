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

#define __FILENUM__ 14 // every file needs to have a unique (8bit) ID

#include <stdint.h>
#include "msg.h"
#include "timer.h"
#include "hal.h"
#include "bus.h"
#include "uart.h"
#include "random.h"
#include "output.h"
#include "frame.h"

#define BUFSIZE 16 // event buffer size (must be power of 2)

#define BUF_MASK (BUFSIZE - 1)
#if (BUFSIZE & BUF_MASK)
	#error BUFSIZE is not a power of 2
#endif



/*************** Output module state ***************/

static struct _global_am_context
{   // codesize saver: place the most frequented on top, cluster them like locally used

    uint8_t modul_addr; // class and module address byte
    // state of the event queue
    uint8_t eventqueue[BUFSIZE]; 
    uint8_t queue_head; // write position, maintained by ISR (timer tick)
    uint8_t queue_tail; // acknowledged up to here, maintained by main loop
    uint8_t queue_wait4ack; // notify message is sent, waiting for main loop
} output;

// assign a bus address
void output_init(uint8_t addr)
{
	output.modul_addr = addr; // class and address
}

// the main loop of the PHC output application
void output_mainloop(void)
{
	while (1)
	{
        if( bus_frame->new == 1){
            printf("new packet adr= %u %u %x \r\n",bus_frame, bus_frame->len, bus_frame->data[0]);
            bus_frame->new = 0;
        }
        /*if( bus_frame != 566 && bus_frame != 0){
            printf("new address: %u\r\n",bus_frame);
        }*/
        hal_watchdog_reset();
	} // while(1)
}

// interrupt context functions for incoming packet processing

// also an ISR, so it won't compete against output_cmd_end()
void output_tick(void)
{
}
