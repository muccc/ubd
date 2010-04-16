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
#ifndef __RC5_HANDLER_H_
#define __RC5_HANDLER_H_

#define RC5_ADDRESS			RC5_ADR_VCR
//#define RC5_USEANY        //define this macro to ignore the RC5 address of the remote

#define KEY_POWER			RC5_POWER
#define KEY_BRIGHTNESS_UP	RC5_BRIGHTNESS_UP
#define KEY_BRIGHTNESS_DOWN	RC5_BRIGHTNESS_DOWN
#define KEY_FASTER			RC5_VOL_UP
#define KEY_SLOWER			RC5_VOL_DOWN
#define KEY_FULL_BRIGHTNESS	RC5_CHAN_UP
#define KEY_ZERO_BRIGHTNESS	RC5_CHAN_DOWN
#define KEY_PAUSE			RC5_PAUSE
#define KEY_SAVE			RC5_RECORD
#define KEY_SLEEP           RC5_MEMO
#define KEY_COLOR_UP        RC5_COLOR_UP
#define KEY_COLOR_DOWN      RC5_COLOR_DOWN
#define KEY_RED             RC5_RED
#define KEY_RED2            RC5_A
#define KEY_GREEN           RC5_GREEN
#define KEY_GREEN2          RC5_B
#define KEY_BLUE            RC5_BLUE
#define KEY_BLUE2           RC5_C
#define KEY_COLOR_FULL      RC5_CONTRAST_UP
#define KEY_COLOR_ZERO      RC5_CONTRAST_DOWN
#define KEY_SCRIPT1			RC5_1
#define KEY_SCRIPT2			RC5_2
#define KEY_SCRIPT3			RC5_3
#define KEY_SCRIPT4			RC5_4
#define KEY_SCRIPT5			RC5_5
#define KEY_SCRIPT6			RC5_6
#define KEY_SCRIPT7			RC5_7
#define KEY_SCRIPT8			RC5_8
#define KEY_SCRIPT9			RC5_9

uint8_t rc5_handler(void);
#endif
