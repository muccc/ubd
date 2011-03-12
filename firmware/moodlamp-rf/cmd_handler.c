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

#if STATIC_SCRIPTS
#include "static_scripts.h"
#include "scripts.h"
#endif
#include "settings.h"
#include "control.h"
#include "adc.h"

void cmd_dark(void)
{
    global_pwm.channels[0].brightness =
                global_pwm.channels[0].target_brightness = 0;
    global_pwm.channels[1].brightness =
                global_pwm.channels[1].target_brightness = 0;
    global_pwm.channels[2].brightness =
                global_pwm.channels[2].target_brightness = 0;
}


void cmd_setscript(void (*execute)(struct thread_t *current_thread), uint16_t position)
{
    script_threads[0].handler.execute = execute;
    script_threads[0].handler.position = position;
    script_threads[0].flags.disabled = 0;
    script_threads[0].handler_stack_offset = 0;
    cmd_dark();
    
    //resets state & oldstate in case fadems was used earlier.
    //TODO: handle in state machine
    global.state=STATE_RUNNING;
    global.oldstate=STATE_RUNNING;
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
    }else if(cmd == CMD_POWER){
        if(global.state != STATE_STANDBY){
            if(global.state == STATE_SLEEP)
                global.oldstate = STATE_RUNNING;
            else
                global.oldstate = global.state;
            //global.state = STATE_ENTERSTANDBY;
            control_standby(0);
        }else{
            global.state = STATE_LEAVESTANDBY;
        }
    }else if(cmd == CMD_SET_SCRIPT){
        if(sizeof(global_playlist)/(sizeof(struct playlist_t)) > param[0])
                cmd_setscript(
                    global_playlist[param[0]].execute, global_playlist[param[0]].position);
    }else if(cmd == CMD_FASTER){
        if(script_threads[0].speed_adjustment < 8){
            script_threads[0].speed_adjustment++;
            respeed(&script_threads[0]);
            //update_brightness();
            //execute_script_threads();
        }

    }else if(cmd == CMD_SLOWER){
        if(script_threads[0].speed_adjustment > -8){
            script_threads[0].speed_adjustment--;
            respeed(&script_threads[0]);
            //update_brightness();
            //execute_script_threads();
        }
    }else if(cmd == CMD_PAUSE){
        if(global.state != STATE_PAUSE)
            global.state = STATE_PAUSE;
        else
            global.state = STATE_RUNNING;
    }else if(cmd == CMD_SAVE){
        settings_save();
    }else if(cmd == CMD_SLEEP){
        global.state = STATE_ENTERSLEEP;
    }else if(cmd == CMD_RED){
        global_pwm.channel_modifier = 0;
    }else if(cmd == CMD_GREEN){
        global_pwm.channel_modifier = 1;
    }else if(cmd == CMD_BLUE){
        global_pwm.channel_modifier = 2;
    }else if(cmd == CMD_COLOR_UP){
        global_pwm.channels[global_pwm.channel_modifier].brightness++;
    }else if(cmd == CMD_COLOR_DOWN){
        global_pwm.channels[global_pwm.channel_modifier].brightness--;
    }else if(cmd == CMD_COLOR_FULL){
        global_pwm.channels[global_pwm.channel_modifier].brightness=255;
    }else if(cmd == CMD_COLOR_ZERO){
        global_pwm.channels[global_pwm.channel_modifier].brightness=0;
    }else if(cmd == CMD_GET_VERSION){
        //strcpy((char *) result,"D=");
        //strcat((char *) result,__DATE__);
        //strcat((char *) result,"H=");

        //strcpy((char *) result, "D="__DATE__"H=");
        //strcat((char *) result,__DATE__);
        if( result == NULL ) return 0;
        sprintf((char *)result,"D="__DATE__" H=%d",global.config);
        return strlen((char *)result);
    }else if(cmd == CMD_GET_STATE){
        if( result == NULL ) return 0;
        result[0] = global.state;
        result[1] = script_threads[0].speed_adjustment;
        result[2] = global_pwm.dim;
        return 3;
    }else if(cmd == CMD_SET_SPEED){
        script_threads[0].speed_adjustment = param[0]; 
    }else if(cmd == CMD_SET_STATE){
        global.state = param[0];
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
    }else if(cmd == CMD_GET_VOLTAGE){
        if( result == NULL ) return 0;
        uint16_t adc = adc_getChannel(6);
        sprintf((char *)result,"V=%u",adc);
        return strlen((char *)result);
    }else if(cmd == CMD_GET_COLOR){
        if( result == NULL ) return 0;
        result[0] = global_pwm.channels[0].brightness;
        result[1] = global_pwm.channels[1].brightness;
        result[2] = global_pwm.channels[2].brightness;
        return 3;
    }
    return 0;
}

