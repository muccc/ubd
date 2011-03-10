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
#include "control.h"
#if RC5_DECODER
#include "rc5.h"
#include "rc5_handler.h"
#endif
#if STATIC_SCRIPTS
#include "static_scripts.h"
#endif
#include "settings.h"
#include "control.h"
#include "pinutils.h"
#include "adc.h"
#include "scripts.h"
#include "leds.h"

#include "ub.h"
#include "ubaddress.h"
#include "ubpacket.h"
#include "usart.h"
/* structs */
volatile struct global_t global = {{0, 0}};
/* prototypes */
//uint16_t main_reset = 0;


void jump_to_bootloader(void)
{
    //uart1_puts("acDRab");
    //cli();
    wdt_enable(WDTO_30MS);
    while(1);
}

void sendstest(void)
{
    static int out = 0;
    static char outp = 'X';
    static int t = 0;

    if( t++ == 1000 ){
        t = 0;
        if( outp == 'B' )
            outp = 'A';
        else
            outp = 'B';
        out = 1;
    }

    if( out ){
        if( ubpacket_acquireUnsolicited(0) ){
            if( !ubpacket_isUnsolicitedDone() ){
                struct ubpacket_t *p = ubpacket_getSendBuffer();
                p->header.src = ubadr_getAddress();
                p->header.dest = UB_ADDRESS_MASTER;
                p->header.flags = UB_PACKET_UNSOLICITED;
                p->header.class = 42;
                p->header.len = 1;
                p->data[0] = outp;
                ubpacket_send();                       
            }else{
                out = 0;
                ubpacket_releaseUnsolicited(0);
            }
        }
    }

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
    //volatile long l;
//    for(l=0;l<1000000;l++);
    DDR_CONFIG_IN(CONFIG1);
    PIN_SET(CONFIG1);
    
    volatile unsigned long l;     
    for(l=0;l<10;l++);

    if( !PIN_HIGH(CONFIG1) ){
        global.config = 30;
    }else{
        global.config = 21;
    }
    
    leds_init();

    if( global.config == 21 ){
        DDR_CONFIG_IN(JUMPER1C1);
        PIN_SET(JUMPER1C1);
    }else if( global.config == 30 ){
        DDR_CONFIG_IN(JUMPER1C2);
        PIN_SET(JUMPER1C2);
        adc_init();
        uint16_t bat = adc_getChannel(6);
        if( bat < ADC_MINBATIDLE ){
            //global.flags.lowbat = 1;
        }
    }
    
    init_pwm();
    uart1_init( UART_BAUD_SELECT(115200,F_CPU));
#if RC5_DECODER
    rc5_init();
#endif

#if STATIC_SCRIPTS
    init_script_threads();
#endif
    settings_read();
    control_init();

#if defined(__AVR_ATmega644P__)
    if((global.config == 21 && !PIN_HIGH(JUMPER1C1)) || (global.config== 30 && !PIN_HIGH(JUMPER1C2)))
        ub_init(UB_SLAVE, UB_RF, UB_RF|UB_RS485);
    else
        ub_init(UB_SLAVE, UB_RS485, UB_RS485);
#elif defined(__AVR_ATmega324P__)
        ub_init(UB_SLAVE, UB_RS485, UB_RS485);
#endif
    /* enable interrupts globally */
    sei();
//    global.state = STATE_RUNNING;
//    global.state = STATE_PAUSE;
//    global.flags.running = 0;
    wdt_enable(WDTO_2S);

    while (1) {
        wdt_reset();
        leds_main();
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

        if(packetbase > 32){
            packetbase = 0;
            ub_tick();

            //sendstest();

            //uint16_t bat = adc_getChannel(6);
            /*if( bat < ADC_MINBATIDLE ){
                global.flags.lowbat = 1;
            }*/
        }
        if( global.flags.lowbat ){
            control_lowbat();
        }

        if( global.flags.timebase ){
            control_tick();
            global.flags.timebase=0;
        }
        /* after the last pwm timeslot, rebuild the timeslot table */
        
        if( global.flags.last_pulse ){
            global.flags.last_pulse = 0;
            //if(global.flags.running)
            //update_pwm_timeslots();
        }
        /* at the beginning of each pwm cycle, call the fading engine and
         * execute all script threads */
        if ( global.flags.new_cycle ) {
            global.flags.new_cycle = 0;
            if( control_faderunning ){
                update_brightness();
            }
            if( global.flags.running ) {
#if STATIC_SCRIPTS
                execute_script_threads();
#endif
            }
            continue;
        }
#if RC5_DECODER
        rc5_handler();
#endif
    }
}
