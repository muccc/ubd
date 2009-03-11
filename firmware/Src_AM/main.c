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

/*! \file main.c
    \brief Entry point of the AM module
*/

#define __FILENUM__ 13 // every file needs to have a unique (8bit) ID

#include <stdint.h>
#include "hal.h"
#include "uart.h"
#include "timer.h"
#include "msg.h"
#include "phc.h"
#include "random.h"
#include "output.h"
#include "softuart.h"

int main(void)
{
	uint8_t addr;
    hal_sysinit();
    hal_watchdog_enable();
    addr = hal_get_addr(); // device address from DIP switches
    addr |= 0x40; // add the device class

    softuart_init();
    softuart_puts( "reset\r\n" );
    uart_init(addr); // timeout affects collision recovery, use address
    rand_seed(((uint16_t)addr << 8) | (uint16_t)addr);
	phc_init(output_cmd_start, output_payload, output_cmd_end);
	timer_init(output_tick, addr); // init with system-wide unique value
	output_init(addr);
    softuart_puts( "main loop\r\n" );

	output_mainloop();
	return 0; // we won't get here
}
