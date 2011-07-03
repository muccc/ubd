/* vim:fdm=marker ts=4 et ai
 * {{{
 *         moodlamp-rf - fnordlicht firmware next generation
 *
 *    for additional information please
 *    see http://blinkenlichts.net/
 *    and http://koeln.ccc.de/prozesse/running/fnordlicht
 *
 * This is a modified version of the fnordlicht
 * (c) by Alexander Neumann <alexander@bumpern.de>
 *     Lars Noschinski <lars@public.noschinski.de>
 *
 * Modifications done by:
 * Kiu
 * Mazzoo
 * Tobias Schneider(schneider@blinkenlichts.net)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * For more information on the GPL, please go to:
 * http://www.gnu.org/copyleft/gpl.html
 }}} */

/* includes */
#include "config.h"

#include <avr/io.h>
#include <stdint.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <string.h>
#include <avr/wdt.h>
#include "common.h"
#include "fnordlicht.h"
#include "pwm.h"
#include "cmd_handler.h"
#include "settings.h"
#include "pinutils.h"
#include "timer0.h"

#include "ub.h"
#include "ubaddress.h"
#include "ubpacket.h"
/* structs */
volatile struct global_t global = {{0, 0}};

void jump_to_bootloader(void)
{
    //uart1_puts("acDRab");
    //cli();
    wdt_enable(WDTO_30MS);
    while(1);
}

unsigned int random_seed __attribute__ ((section (".noinit")));

/** main function
 */
int main(void) {
//    SPCR &= ~(1<<SPE);
//    TIMSK0 &= ~(1<<TOIE1);
    wdt_disable();
    /* Clear WDRF in MCUSR */
    MCUSR &= ~(1<<WDRF);
    /* Write logical one to WDCE and WDE */
    /* Keep old prescaler setting to prevent unintentional time-out */
    WDTCSR |= (1<<WDCE) | (1<<WDE);
    /* Turn off WDT */
    WDTCSR = 0x00;
    timer0_init();
    init_pwm();
    settings_read();
#ifdef UB_ENABLERF 
    DDRB |= (1<<PB4);   // SS has to be an output or SPI will switch to slave mode
    PORTB &= ~(1<<PB4);
    ub_init(UB_SLAVE, UB_RF, 0);
#endif
#ifdef UB_ENABLERS485
    ub_init(UB_SLAVE, UB_RS485, 0);
#endif
    /* enable interrupts globally */
    sei();
    wdt_enable(WDTO_2S);

    while (1) {
        wdt_reset();
        ub_process();
        if( ubpacket_gotPacket() ){
            struct ubpacket_t * out = ubpacket_getSendBuffer();
            uint8_t len = cmd_interpret(ubpacket_getIncomming()->data,
                                        out->data);
            if( !(ubpacket_getIncomming()->header.flags & UB_PACKET_NOACK) ){
                out->header.len = len; 
                out->header.class = 23;
            }
            ubpacket_processed();
        }

        if(timebase ){
            timebase = 0;
            ub_tick();
        }

        /* at the beginning of each pwm cycle, call the fading engine and
         * execute all script threads */
        if ( global.flags.new_cycle ) {
            global.flags.new_cycle = 0;
                update_brightness();
        }
    }
}
