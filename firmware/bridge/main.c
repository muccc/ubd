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
#include "bus.h"
#include "random.h"
#include "output.h"
#include "uart.h"

#define UART_BAUDRATE  115200

int uart_putc_file(char c, FILE *stream);
static FILE mystdout = FDEV_SETUP_STREAM(uart_putc_file, NULL, _FDEV_SETUP_WRITE);

int uart_putc_file(char c, FILE *stream)
{
    uart1_putc(c);
    return 0;
}

int main(void)
{
	uint8_t addr;
    hal_sysinit();

    uart1_init( UART_BAUD_SELECT(UART_BAUDRATE,F_CPU));
    stdout = &mystdout;
    printf("booting\r\n");
    
    addr = hal_get_addr(); // device address from DIP switches
    addr |= 0x40; // add the device class
    
    hal_watchdog_enable();
    uart_init(addr); // timeout affects collision recovery, use address
    rand_seed(((uint16_t)addr << 8) | (uint16_t)addr);
	bus_init();
	timer_init(output_tick, addr); // init with system-wide unique value
	output_init(addr);
	output_mainloop();
	return 0; // we won't get here
}
