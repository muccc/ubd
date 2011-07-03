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
#include <string.h>
#include <stdio.h>

#include "common.h"
#include "fnordlicht.h"
#include "pwm.h"
#include "cmd_handler.h"

#include "settings.h"
#include "control.h"

void cmd_dark(void)
{
    global_pwm.channels[0].brightness =
                global_pwm.channels[0].target_brightness = 0;
    global_pwm.channels[1].brightness =
                global_pwm.channels[1].target_brightness = 0;
    global_pwm.channels[2].brightness =
                global_pwm.channels[2].target_brightness = 0;
}


uint8_t cmd_interpret(uint8_t * cmd, uint8_t * result)
{
    return cmd_handler(cmd[0],cmd+1,result);
}

uint8_t cmd_handler(uint8_t cmd, uint8_t * param, uint8_t * result)
{
   if(cmd == CMD_BRIGHTNESS_DOWN){
        if(global_pwm.dim > 0){
            global_pwm.dim--;
        }
    }else if(cmd == CMD_BRIGHTNESS_UP){
        if(global_pwm.dim < 255){
            global_pwm.dim++;
        }
    }else if(cmd == CMD_FULL_BRIGHTNESS){
        global_pwm.dim=255;
    }else if(cmd == CMD_ZERO_BRIGHTNESS){
        global_pwm.dim=0;
    }else if(cmd == CMD_SAVE){
        settings_save();
    }else if(cmd == CMD_SET_BRIGHTNESS){
        global_pwm.dim = param[0];
    }else if(cmd == CMD_SET_COLOR){
        control_setColor(param[0],param[1],param[2]);
    }else if(cmd == CMD_RESET){
        jump_to_bootloader();
    }else if(cmd == CMD_FADE){
        uint16_t speed = (param[3]<<8)+param[4];
        control_fade(param[0],param[1],param[2],speed);
    }else if(cmd == CMD_FADEMS){
        uint16_t time = (param[3]<<8)+param[4];
        control_fadems(param[0],param[1],param[2],time);
    }else if(cmd == CMD_FADEMSALT){
        uint16_t time = (param[3]<<8)+param[4];
        control_fademsalt(param[0],param[1],param[2],time);
    }else if(cmd == CMD_GET_COLOR){
        if( result == NULL ) return 0;
        result[0] = global_pwm.channels[0].brightness;
        result[1] = global_pwm.channels[1].brightness;
        result[2] = global_pwm.channels[2].brightness;
        return 3;
    }else if(cmd == CMD_FLASH){
        uint16_t time = (param[3]<<8)+param[4];
        control_flash(param[0],param[1],param[2],time);
    }
    return 0;
}

