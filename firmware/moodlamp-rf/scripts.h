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

#if STATIC_SCRIPTS

#include "static_scripts.h"
#include "avr/pgmspace.h"
/* testscript */
static const uint8_t testscript_flash1[] PROGMEM = {
    MACRO_WAIT(_BV(CHANNEL_RED)),
    MACRO_SET_CHANNEL(CHANNEL_BLUE, 0xff),
    MACRO_WAIT(_BV(CHANNEL_RED)),
    MACRO_SET_CHANNEL(CHANNEL_BLUE, 0),
    MACRO_JUMP(-4),
    MACRO_STOP()
    };

static const uint8_t testscript_flash2[] PROGMEM = {
    MACRO_FADE_CHANNEL(CHANNEL_RED, 255, 0x100),
    MACRO_WAIT(_BV(CHANNEL_RED)),
    MACRO_FADE_CHANNEL(CHANNEL_RED, 100, 0x100),
    MACRO_WAIT(_BV(CHANNEL_RED)),
    MACRO_JUMP(-4),
    };


static const uint8_t testscript_eeprom[] EEMEM = {
    MACRO_FADE_CHANNEL(CHANNEL_GREEN, 255, 0xf00),
    MACRO_WAIT(_BV(CHANNEL_GREEN)),
    MACRO_FADE_CHANNEL(CHANNEL_GREEN, 0, 0x100),
    MACRO_WAIT(_BV(CHANNEL_GREEN)),
    MACRO_JUMP(-4)
    };

static const uint8_t colorchange_rnd[] PROGMEM = {

    MACRO_RND_CHANNEL(CHANNEL_RED  , 0x22, 0x02),
    MACRO_WAIT(_BV(CHANNEL_RED)|_BV(CHANNEL_GREEN)|_BV(CHANNEL_BLUE)),
    MACRO_RND_CHANNEL(CHANNEL_GREEN, 0x22, 0x02),
    MACRO_WAIT(_BV(CHANNEL_RED)|_BV(CHANNEL_GREEN)|_BV(CHANNEL_BLUE)),
    MACRO_RND_CHANNEL(CHANNEL_BLUE , 0x22, 0x02),
    MACRO_WAIT(_BV(CHANNEL_RED)|_BV(CHANNEL_GREEN)|_BV(CHANNEL_BLUE)),

    MACRO_JUMP(-6),
    };


static const uint8_t colorchange_red[] PROGMEM = {
    //MACRO_SET_CHANNEL(CHANNEL_RED, 0),
    //MACRO_SET_CHANNEL(CHANNEL_GREEN, 0),
    //MACRO_SET_CHANNEL(CHANNEL_BLUE, 0),
    //MACRO_SLEEP(1),
    //MACRO_JUMP(-1),

    MACRO_FADE_CHANNEL(CHANNEL_RED, 255, 0x200),
    MACRO_WAIT(_BV(CHANNEL_RED)|_BV(CHANNEL_GREEN)|_BV(CHANNEL_BLUE)),
    MACRO_FADE_CHANNEL(CHANNEL_RED, 0, 0x040),
    MACRO_FADE_CHANNEL(CHANNEL_GREEN, 255, 0x040),
    MACRO_WAIT(_BV(CHANNEL_RED)|_BV(CHANNEL_GREEN)|_BV(CHANNEL_BLUE)),
    MACRO_FADE_CHANNEL(CHANNEL_GREEN, 0, 0x040),
    MACRO_FADE_CHANNEL(CHANNEL_BLUE, 255, 0x040),
    MACRO_WAIT(_BV(CHANNEL_RED)|_BV(CHANNEL_GREEN)|_BV(CHANNEL_BLUE)),
    MACRO_FADE_CHANNEL(CHANNEL_BLUE, 0, 0x040),
    MACRO_FADE_CHANNEL(CHANNEL_RED, 255, 0x040),
    MACRO_JUMP(-9)
    };

static const uint8_t off[] PROGMEM = {
    MACRO_SET_CHANNEL(CHANNEL_RED, 0),
    MACRO_SET_CHANNEL(CHANNEL_GREEN, 0),
    MACRO_SET_CHANNEL(CHANNEL_BLUE, 0),
    MACRO_SLEEP(125),
    MACRO_JUMP(-3)
    };

static const uint8_t colorchange_red_blue[] PROGMEM = {
    MACRO_SET_CHANNEL(CHANNEL_RED, 0),
    MACRO_SET_CHANNEL(CHANNEL_GREEN, 0),
    MACRO_SET_CHANNEL(CHANNEL_BLUE, 0),
    MACRO_FADE_CHANNEL(CHANNEL_RED, 255, 0x200),

    MACRO_WAIT(_BV(CHANNEL_RED)|_BV(CHANNEL_BLUE)),
    MACRO_FADE_CHANNEL(CHANNEL_RED, 0, 0x040),
    MACRO_FADE_CHANNEL(CHANNEL_BLUE, 255, 0x040),
    MACRO_WAIT(_BV(CHANNEL_RED)|_BV(CHANNEL_BLUE)),
    MACRO_FADE_CHANNEL(CHANNEL_BLUE, 0, 0x040),
    MACRO_FADE_CHANNEL(CHANNEL_RED, 255, 0x040),
    MACRO_JUMP(-6)
    };

static const uint8_t green_flash[] PROGMEM = {
    MACRO_SET_CHANNEL(CHANNEL_RED, 0),
    MACRO_SET_CHANNEL(CHANNEL_GREEN, 0),
    MACRO_SET_CHANNEL(CHANNEL_BLUE, 0),
	MACRO_SLEEP(125),
    MACRO_FADE_CHANNEL(CHANNEL_GREEN, 255, 0xf00),
    MACRO_WAIT(_BV(CHANNEL_GREEN)),
    MACRO_FADE_CHANNEL(CHANNEL_GREEN, 0, 0x100),
    MACRO_WAIT(_BV(CHANNEL_GREEN)),
	MACRO_JUMP(-4),
    MACRO_STOP()
    };

static const uint8_t green_blink[] PROGMEM = {
    MACRO_SET_CHANNEL(CHANNEL_RED, 0),
    MACRO_SET_CHANNEL(CHANNEL_BLUE, 0),
    MACRO_SET_CHANNEL(CHANNEL_GREEN, 150),
    MACRO_SLEEP(125),
    MACRO_SET_CHANNEL(CHANNEL_GREEN, 0),
    MACRO_SLEEP(125),
    MACRO_JUMP(-4)
    };

static const uint8_t red_blink[] PROGMEM = {
    MACRO_SET_CHANNEL(CHANNEL_GREEN, 0),
    MACRO_SET_CHANNEL(CHANNEL_BLUE, 0),
    MACRO_SET_CHANNEL(CHANNEL_RED, 150),
    MACRO_SLEEP(125),
    MACRO_SET_CHANNEL(CHANNEL_RED, 0),
    MACRO_SLEEP(125),
    MACRO_JUMP(-4)
    };
   
static const uint8_t police[] PROGMEM = {
    MACRO_SET_CHANNEL(CHANNEL_RED, 0),
    MACRO_SET_CHANNEL(CHANNEL_GREEN, 0),
    MACRO_SET_CHANNEL(CHANNEL_BLUE, 0),
//    MACRO_SLEEP(12),
    MACRO_FADE_CHANNEL(CHANNEL_BLUE, 255, 0x4000),
    MACRO_WAIT(_BV(CHANNEL_BLUE)),
    MACRO_SLEEP(2),
    MACRO_FADE_CHANNEL(CHANNEL_BLUE, 0, 0x4000),
    MACRO_WAIT(_BV(CHANNEL_BLUE)),
    MACRO_SLEEP(6),
    MACRO_FADE_CHANNEL(CHANNEL_BLUE, 255, 0x4000),
    MACRO_WAIT(_BV(CHANNEL_BLUE)),
    MACRO_SLEEP(2),
    MACRO_FADE_CHANNEL(CHANNEL_BLUE, 0, 0x4000),
    MACRO_WAIT(_BV(CHANNEL_BLUE)),
    MACRO_SLEEP(50),
    MACRO_JUMP(-12),
    MACRO_STOP()
    };

static const uint8_t blinken[] PROGMEM = {
    MACRO_SET_CHANNEL(CHANNEL_RED, 255),
    MACRO_SET_CHANNEL(CHANNEL_GREEN, 0),
    MACRO_SET_CHANNEL(CHANNEL_BLUE, 0),
    MACRO_SLEEP(125),
    MACRO_SET_CHANNEL(CHANNEL_RED, 0),
    MACRO_SET_CHANNEL(CHANNEL_GREEN, 255),
    MACRO_SET_CHANNEL(CHANNEL_BLUE, 0),
    MACRO_SLEEP(125),
    MACRO_SET_CHANNEL(CHANNEL_RED, 0),
    MACRO_SET_CHANNEL(CHANNEL_GREEN, 0),
    MACRO_SET_CHANNEL(CHANNEL_BLUE, 255),
    MACRO_SLEEP(125),
    MACRO_SET_CHANNEL(CHANNEL_RED, 128),
    MACRO_SET_CHANNEL(CHANNEL_GREEN, 0),
    MACRO_SET_CHANNEL(CHANNEL_BLUE, 0),
    MACRO_SLEEP(125),
    MACRO_SET_CHANNEL(CHANNEL_RED, 0),
    MACRO_SET_CHANNEL(CHANNEL_GREEN, 128),
    MACRO_SET_CHANNEL(CHANNEL_BLUE, 0),
    MACRO_SLEEP(125),
    MACRO_SET_CHANNEL(CHANNEL_RED, 0),
    MACRO_SET_CHANNEL(CHANNEL_GREEN, 0),
    MACRO_SET_CHANNEL(CHANNEL_BLUE, 128),
    MACRO_SLEEP(125),
    MACRO_JUMP(-24)
    };
static const struct playlist_t global_playlist[] = {
    {&memory_handler_flash, (uint16_t) &green_flash},
    {&memory_handler_flash, (uint16_t) &blinken},
    {&memory_handler_flash, (uint16_t) &colorchange_rnd},
    {&memory_handler_flash, (uint16_t) &colorchange_red},
    {&memory_handler_flash, (uint16_t) &colorchange_red_blue},
    {&memory_handler_flash, (uint16_t) &green_blink},
    {&memory_handler_flash, (uint16_t) &police},
    {&memory_handler_flash, (uint16_t) &red_blink},
    {&memory_handler_flash, (uint16_t) &off}
};
#endif
