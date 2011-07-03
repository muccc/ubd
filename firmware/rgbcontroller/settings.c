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
#include <avr/eeprom.h>
#include <string.h>

#include "fnordlicht.h"
#include "settings.h"
#include "pwm.h"
#include "control.h"

struct settings_record_t global_settings_record EEMEM = {1,1};
struct global_pwm_t global_pwm_record EEMEM;
struct timeslots_t pwm_record EEMEM;
struct global_t global_record EEMEM;
struct settings_record_t global_settings;

void settings_save(void)
{
    const void * temp;
    global_settings.firstboot = 0;
    eeprom_write_block(&global_settings, &global_settings_record,sizeof(global_settings)); 
    temp =(const void *) &global_pwm;       //Just to avoid compiler warnings
    eeprom_write_block(
            temp,
            &global_pwm_record,
            sizeof(global_pwm)
    );
    eeprom_write_block(&pwm,&pwm_record,sizeof(pwm));
    temp = (const void *) &global;
    eeprom_write_block(/*(struct global_t *)&global*/temp,&global_record,sizeof(global));

}

void settings_read(void)
{
    void * temp;
    eeprom_read_block(&global_settings, &global_settings_record,sizeof(global_settings));
    if(global_settings.firstboot){
        global_pwm.dim = 255;
        global_pwm.channels[0].brightness = 250;
        global_pwm.channels[0].target_brightness = 250;
        global.state = STATE_REMOTE;
    }else{
        temp = (void *) &global_pwm;
        eeprom_read_block(/*(void *)&global_pwm*/temp,&global_pwm_record,sizeof(global_pwm));
        eeprom_read_block(&pwm,&pwm_record,sizeof(pwm));
        temp = (void *) &global;
        eeprom_read_block(/*(struct global_t *)&global*/temp,&global_record,sizeof(global));
    }
}

