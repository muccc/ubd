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
#ifndef __CMD_HANDLER_H_
#define __CMD_HANDLER_H_

#include "static_scripts.h"
#define CMD_POWER               0x10
#define CMD_BRIGHTNESS_UP       0x11
#define CMD_BRIGHTNESS_DOWN     0x12
#define CMD_FASTER              0x13
#define CMD_SLOWER              0x14
#define CMD_FULL_BRIGHTNESS     0x15
#define CMD_ZERO_BRIGHTNESS     0x16
#define CMD_PAUSE               0x17
#define CMD_SAVE                0x18
#define CMD_SLEEP               0x19
#define CMD_COLOR_UP            0x1A
#define CMD_COLOR_DOWN          0x1B
#define CMD_RED                 0x1C
#define CMD_GREEN               0x1D
#define CMD_BLUE                0x1E
#define CMD_COLOR_FULL          0x1F
#define CMD_COLOR_ZERO          0x20
#define CMD_SET_SCRIPT          0x21
#define CMD_GET_VERSION         'V'
#define CMD_GET_STATE           'G'
#define CMD_SET_SPEED           's'
#define CMD_SET_BRIGHTNESS      'D'
#define CMD_SET_STATE           'S'
#define CMD_SET_COLOR           'C'
#define CMD_RESET               'r'
#define CMD_SET_NAME            'N'
#define CMD_FADE                'F'
#define CMD_FADEMS              'M'
#define CMD_FADEMSALT           'T'
#define CMD_GET_VOLTAGE         'K'
#define CMD_GET_COLOR           'g'
#define CMD_FLASH               'f'


uint8_t cmd_interpret(uint8_t * cmd, uint8_t * result);
uint8_t cmd_handler(uint8_t cmd, uint8_t * param, uint8_t * result);
void cmd_setscript(void (*execute)(struct thread_t *current_thread), uint16_t position);
#endif
