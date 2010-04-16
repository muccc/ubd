/* vim:fdm=marker ts=4 et ai
 * {{{
 *         moodlamp-ng - fnordlicht firmware next generation
 *
 *    for additional information please
 *    see http://blinkenlichts.net/
 *    and http://koeln.ccc.de/prozesse/running/fnordlicht
 *
 * This is a modified version of the fnordlicht
 * (c) by Alexander Neumann <alexander@bumpern.de>
 *     Lars Noschinski <lars@public.noschinski.de>
 *
 * Modifications done by Tobias Schneider(schneider@blinkenlichts.net)
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

#include "config.h"

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include "common.h"
#include "fnordlicht.h"
#include "pwm.h"
#include "control.h"

#if RC5_DECODER
#include "rc5.h"
#include "rc5_handler.h"
#endif

#include "cmd_handler.h"

#if STATIC_SCRIPTS
#include "static_scripts.h"
#include "scripts.h"
#endif
#include "settings.h"

//#define NULL (void*) 0
#define REMOTE1_ADR         6     
#define REMOTE2_ADR         7

//unsigned char remotemap[10] = {REMOTE1_ADR, REMOTE2_ADR};

//unsigned char remotecommands[2][30] = {{2,1,3,3,5},{3,4,6,1}};

#if RC5_DECODER

uint8_t rc5_handler(void)
{
    cli();
    int16_t rc5d =  rc5_data;
    rc5_data = 0;
    sei();
    if(!rc5_checkRC5(rc5d))
        return 1;
    uint8_t rc5adr = rc5d >> 6 & 0x1F;
    uint8_t rc5cmd = (rc5d & 0x3F) | (~rc5d >> 7 & 0x40);

/*    uart1_puts("acDRC");
    uart1_putc(rc5adr);
    uart1_putc(rc5cmd);
    uart1_puts("ab");*/


#ifndef RC5_USEANY
    if(rc5adr != RC5_ADDRESS){
        return 1;
    }
#endif

    //unsigned char cmd = remotecommands[remotemap[rc5adr]][rc5cmd];

    if(global.state == STATE_STANDBY && rc5cmd != RC5_POWER && rc5cmd != RC5_RECORD)
        return 1;

    if(rc5cmd == KEY_BRIGHTNESS_DOWN){
        cmd_handler(CMD_BRIGHTNESS_DOWN,NULL,NULL);
    }else if(rc5cmd == KEY_BRIGHTNESS_UP){
        cmd_handler(CMD_BRIGHTNESS_UP,NULL,NULL);
    }else if(rc5cmd == KEY_FULL_BRIGHTNESS){
        cmd_handler(CMD_FULL_BRIGHTNESS,NULL,NULL);   
    }else if(rc5cmd == KEY_ZERO_BRIGHTNESS){
        cmd_handler(CMD_ZERO_BRIGHTNESS,NULL,NULL);
    }else if(rc5cmd == KEY_POWER){
        cmd_handler(CMD_POWER,NULL,NULL);
    }else if(rc5cmd > 0 && rc5cmd < 10){
        rc5cmd--;       //Remote starts counting by one
        cmd_handler(CMD_SET_SCRIPT,&rc5cmd,NULL);
    }else if(rc5cmd == KEY_FASTER){
        cmd_handler(CMD_FASTER,NULL,NULL);
    }else if(rc5cmd == KEY_SLOWER){
        cmd_handler(CMD_SLOWER,NULL,NULL);
    }else if(rc5cmd == KEY_PAUSE){
        cmd_handler(CMD_PAUSE,NULL,NULL);
    }else if(rc5cmd == KEY_SAVE){
        cmd_handler(CMD_SAVE,NULL,NULL);
    }else if(rc5cmd == KEY_SLEEP){
        cmd_handler(CMD_SLEEP,NULL,NULL);
    }else if(rc5cmd == KEY_RED || rc5cmd == KEY_RED2){
        cmd_handler(CMD_RED,NULL,NULL);
    }else if(rc5cmd == KEY_GREEN || rc5cmd == KEY_GREEN2){
        cmd_handler(CMD_GREEN,NULL,NULL);
    }else if(rc5cmd == KEY_BLUE || rc5cmd == KEY_BLUE2){
        cmd_handler(CMD_BLUE,NULL,NULL);
    }else if(rc5cmd == KEY_COLOR_UP){
        cmd_handler(CMD_COLOR_UP,NULL,NULL);
    }else if(rc5cmd == KEY_COLOR_DOWN){
        cmd_handler(CMD_COLOR_DOWN,NULL,NULL);
    }else if(rc5cmd == KEY_COLOR_FULL){
        cmd_handler(CMD_COLOR_FULL,NULL,NULL);
    }else if(rc5cmd == KEY_COLOR_ZERO){
        cmd_handler(CMD_COLOR_ZERO,NULL,NULL);
    }

    return 0;
}
#endif

