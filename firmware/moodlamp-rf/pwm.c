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
/* includes */
#include "config.h"

#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "common.h"
#include "fnordlicht.h"
#include "pwm.h"

/* TYPES AND PROTOTYPES */


static inline void prepare_next_timeslot(void);

/* GLOBAL VARIABLES */
#ifdef FAST_PWM
/* timer top values for 256 brightness levels (stored in flash) */
static const uint16_t timeslot_table[] PROGMEM = {
 1 ,
 4 ,
 9 ,
 16 ,
 25 ,
 35 ,
 48 ,
 63 ,
 80 ,
 98 ,
 119 ,
 142 ,
 166 ,
 193 ,
 221 ,
 252 ,
 284 ,
 319 ,
 355 ,
 394 ,
 434 ,
 476 ,
 521 ,
 567 ,
 615 ,
 665 ,
 718 ,
 772 ,
 828 ,
 886 ,
 946 ,
 1008 ,
 1072 ,
 1138 ,
 1206 ,
 1276 ,
 1347 ,
 1421 ,
 1497 ,
 1575 ,
 1655 ,
 1736 ,
 1820 ,
 1905 ,
 1993 ,
 2083 ,
 2174 ,
 2268 ,
 2363 ,
 2461 ,
 2560 ,
 2661 ,
 2765 ,
 2870 ,
 2977 ,
 3087 ,
 3198 ,
 3311 ,
 3426 ,
 3543 ,
 3662 ,
 3783 ,
 3906 ,
 4031 ,
 4158 ,
 4287 ,
 4418 ,
 4551 ,
 4686 ,
 4823 ,
 4962 ,
 5102 ,
 5245 ,
 5390 ,
 5536 ,
 5685 ,
 5836 ,
 5988 ,
 6143 ,
 6299 ,
 6458 ,
 6618 ,
 6780 ,
 6945 ,
 7111 ,
 7279 ,
 7450 ,
 7622 ,
 7796 ,
 7972 ,
 8150 ,
 8331 ,
 8513 ,
 8697 ,
 8883 ,
 9071 ,
 9261 ,
 9453 ,
 9647 ,
 9842 ,
 10040 ,
 10240 ,
 10442 ,
 10646 ,
 10851 ,
 11059 ,
 11269 ,
 11480 ,
 11694 ,
 11909 ,
 12127 ,
 12346 ,
 12568 ,
 12791 ,
 13017 ,
 13244 ,
 13473 ,
 13705 ,
 13938 ,
 14173 ,
 14410 ,
 14649 ,
 14891 ,
 15134 ,
 15379 ,
 15626 ,
 15875 ,
 16126 ,
 16379 ,
 16634 ,
 16890 ,
 17149 ,
 17410 ,
 17673 ,
 17938 ,
 18204 ,
 18473 ,
 18744 ,
 19016 ,
 19291 ,
 19568 ,
 19846 ,
 20127 ,
 20409 ,
 20694 ,
 20980 ,
 21268 ,
 21559 ,
 21851 ,
 22145 ,
 22442 ,
 22740 ,
 23040 ,
 23342 ,
 23646 ,
 23952 ,
 24260 ,
 24570 ,
 24882 ,
 25196 ,
 25512 ,
 25830 ,
 26150 ,
 26472 ,
 26796 ,
 27122 ,
 27449 ,
 27779 ,
 28111 ,
 28444 ,
 28780 ,
 29118 ,
 29457 ,
 29799 ,
 30142 ,
 30488 ,
 30835 ,
 31185 ,
 31536 ,
 31889 ,
 32245 ,
 32602 ,
 32961 ,
 33322 ,
 33686 ,
 34051 ,
 34418 ,
 34787 ,
 35158 ,
 35531 ,
 35906 ,
 36283 ,
 36662 ,
 37043 ,
 37426 ,
 37810 ,
 38197 ,
 38586 ,
 38977 ,
 39369 ,
 39764 ,
 40161 ,
 40559 ,
 40960 ,
 41363 ,
 41767 ,
 42174 ,
 42582 ,
 42992 ,
 43405 ,
 43819 ,
 44236 ,
 44654 ,
 45074 ,
 45496 ,
 45921 ,
 46347 ,
 46775 ,
 47205 ,
 47637 ,
 48071 ,
 48507 ,
 48945 ,
 49385 ,
 49827 ,
 50271 ,
 50717 ,
 51165 ,
 51614 ,
 52066 ,
 52520 ,
 52976 ,
 53433 ,
 53893 ,
 54354 ,
 54818 ,
 55284 ,
 55751 ,
 56221 ,
 56692 ,
 57165 ,
 57641 ,
 58118 ,
 58598 ,
 59079 ,
 59562 ,
 60047 ,
 60535 ,
 61024 ,
 61515 ,
 62008 ,
 62503 ,
 63000 ,
 63499
};

#else

static const uint16_t timeslot_table[] PROGMEM =
/*{{{*/ {
      2,     8,    18,    31,    49,    71,    96,   126,
    159,   197,   238,   283,   333,   386,   443,   504,
    569,   638,   711,   787,   868,   953,  1041,  1134,
   1230,  1331,  1435,  1543,  1655,  1772,  1892,  2016,
   2144,  2276,  2411,  2551,  2695,  2842,  2994,  3150,
   3309,  3472,  3640,  3811,  3986,  4165,  4348,  4535,
   4726,  4921,  5120,  5323,  5529,  5740,  5955,  6173,
   6396,  6622,  6852,  7087,  7325,  7567,  7813,  8063,
   8317,  8575,  8836,  9102,  9372,  9646,  9923, 10205,
  10490, 10779, 11073, 11370, 11671, 11976, 12285, 12598,
  12915, 13236, 13561, 13890, 14222, 14559, 14899, 15244,
  15592, 15945, 16301, 16661, 17025, 17393, 17765, 18141,
  18521, 18905, 19293, 19685, 20080, 20480, 20884, 21291,
  21702, 22118, 22537, 22960, 23387, 23819, 24254, 24693,
  25135, 25582, 26033, 26488, 26946, 27409, 27876, 28346,
  28820, 29299, 29781, 30267, 30757, 31251, 31750, 32251,
  32757, 33267, 33781, 34299, 34820, 35346, 35875, 36409,
  36946, 37488, 38033, 38582, 39135, 39692, 40253, 40818,
  41387, 41960, 42537, 43117, 43702, 44291, 44883, 45480,
  46080, 46684, 47293, 47905, 48521, 49141, 49765, 50393,
  51025, 51661, 52300, 52944, 53592, 54243, 54899, 55558,
  56222, 56889, 57560, 58235, 58914, 59598, 60285, 60975,
  61670, 62369, 63072, 63779,   489,  1204,  1922,  2645,
   3371,  4101,  4836,  5574,  6316,  7062,  7812,  8566,
   9324, 10085, 10851, 11621, 12394, 13172, 13954, 14739,
  15528, 16322, 17119, 17920, 18725, 19534, 20347, 21164,
  21985, 22810, 23638, 24471, 25308, 26148, 26993, 27841,
  28693, 29550, 30410, 31274, 32142, 33014, 33890, 34770,
  35654, 36542, 37433, 38329, 39229, 40132, 41040, 41951,
  42866, 43786, 44709, 45636, 46567, 47502, 48441, 49384,
  50331, 51282, 52236, 53195, 54158, 55124, 56095, 57069,
  58047, 59030, 60016, 61006, 62000, 62998 }; /*}}}*/
#endif
/* pwm timeslots (the top values and masks for the timer1 interrupt) */
struct timeslots_t pwm;
volatile struct global_pwm_t global_pwm;

/* FUNCTIONS AND INTERRUPTS */

/** init timer 1 */
inline void init_timer1(void)
/*{{{*/ {
    /* no prescaler, CTC mode */
    TCCR1B = _BV(CS10) | _BV(WGM12);
    //TCCR1B = _BV(CS12) | _BV(CS10) | _BV(WGM12);

    /* enable timer1 overflow (=output compare 1a)
     * and output compare 1b interrupt */
    _TIMSK_TIMER1 |= _BV(OCIE1A) | _BV(OCIE1B);

    /* set TOP for CTC mode */
    OCR1A = 64000;

    /* load initial delay, trigger an overflow */
    OCR1B = 65000;
    //DDRD |= (1<<PD6)|(1<<PD5)|(1<<PD4)|(1<<PD3);
}

/* }}} */

/** init pwm */
inline void init_pwm(void)
/*{{{*/ {
    uint8_t i;

    init_timer1();
    global_pwm.dim = 255;
    global_pwm.channel_modifier = 0;
    for (i=0; i<3; i++) {
        global_pwm.channels[i].brightness = 0;
        global_pwm.channels[i].target_brightness = 0;
        global_pwm.channels[i].speed = 0x0100;
        global_pwm.channels[i].flags.target_reached = 0;
        global_pwm.channels[i].remainder = 0;
        global_pwm.channels[i].mask = _BV(i);
    }

    update_pwm_timeslots();
    /* set all channels high -> leds off */
#if LED_PORT_INVERT
    LED_PORT |= 7;
#else
    LED_PORT &= ~7;
#endif

    /* configure Px0-Px2 as outputs */
    LED_PORT_DDR |= 7;
}

/* }}} */

static inline uint8_t scalevalue(uint8_t v, uint8_t s)
{
    if(v == 0)
        return 0;
    if(v == 255 && s == 255)
        return 255;
    uint16_t t = v * s;
    v = t/256;              //much faster than /255 but produces a small error
    return v;
}

/** update pwm timeslot table */
void update_pwm_timeslots(void)
/*{{{*/ {
    uint8_t sorted[PWM_CHANNELS] = { 0, 1, 2 };
    uint8_t i, j;
    uint8_t mask = 0;
    uint8_t last_brightness = 0;
    
    /*static uint8_t old[3];
    static uint8_t olddim = 0;
    if(old[0] == global_pwm.channels[0].brightness &&
            old[1] == global_pwm.channels[1].brightness &&
            old[2] == global_pwm.channels[2].brightness &&
            olddim == global_pwm.dim){
        pwm.index = 0;
        pwm.next_bitmask = 0;
        return;
    }
    old[0] = global_pwm.channels[0].brightness;
    old[1] = global_pwm.channels[1].brightness;
    old[2] = global_pwm.channels[2].brightness;
    olddim = global_pwm.dim;
    */
    for(i=0;i<PWM_CHANNELS;i++){
#ifdef PWM_USESCALE
        global_pwm.channels[i].brightness_scale = scalevalue(global_pwm.channels[i].brightness,global_pwm.dim);
#else
        global_pwm.channels[i].brightness_scale = global_pwm.channels[i].brightness;
#endif
    }
        
    /* sort channels according to the current brightness */
    for (i=0; i<PWM_CHANNELS; i++) {
        for (j=i+1; j<PWM_CHANNELS; j++) {
            if (global_pwm.channels[sorted[j]].brightness_scale < global_pwm.channels[sorted[i]].brightness_scale) {
                uint8_t temp;

                temp = sorted[i];
                sorted[i] = sorted[j];
                sorted[j] = temp;
            }
        }
    }

    /* timeslot index */
    j = 0;
    uint8_t index;

    /* calculate timeslots and masks */
    for (i=0; i < PWM_CHANNELS; i++) {
        index = sorted[i];
        /* check if a timeslot is needed */
        if (global_pwm.channels[index].brightness_scale > 0 && global_pwm.channels[index].brightness_scale < 255) {

#ifndef FAST_PWM
            /* if the next timeslot will be after the middle of the pwm cycle, insert the middle interrupt */
            if (last_brightness < 181 && global_pwm.channels[sorted[i]].brightness_scale >= 181) {
                /* middle interrupt: top 65k and mask 0xff */
                pwm.slots[j].top = 65000;
                pwm.slots[j].mask = 0;
                j++;
            }
#endif
            /* insert new timeslot if brightness is new */
            if (global_pwm.channels[index].brightness_scale > last_brightness) {

                /* remember mask and brightness_scale for next timeslot */
                mask |= global_pwm.channels[index].mask;
                last_brightness = global_pwm.channels[index].brightness_scale;

                /* allocate new timeslot */
                pwm.slots[j].top = pgm_read_word(&timeslot_table[global_pwm.channels[index].brightness_scale - 1 ]);
                pwm.slots[j].mask = mask;
                j++;
            } else {
            /* change mask of last-inserted timeslot */
                mask |= global_pwm.channels[index].mask;
                pwm.slots[j-1].mask = mask;
            }
        }
    }
#ifndef FAST_PWM
    /* if all interrupts happen before the middle interrupt, insert it here */
    if (last_brightness < 181) {
        /* middle interrupt: top 65k and mask 0xff */
        pwm.slots[j].top = 65000;
        pwm.slots[j].mask = 0;
        j++;
    }
#endif
    /* reset pwm structure */
    pwm.index = 0;
    pwm.count = j;
    pwm.next_bitmask = 0;

    /* calculate initial bitmask */
    pwm.initial_bitmask = 0xff;
    for (i=0; i < PWM_CHANNELS; i++)
        if (global_pwm.channels[i].brightness_scale > 0)
            pwm.initial_bitmask &= ~global_pwm.channels[i].mask;
} /*}}}*/

/** fade any channels not already at their target brightness */
void update_brightness(void)
/*{{{*/ {
    uint8_t i;

    //PORTC ^= (1<<PC1);
    /* iterate over the channels */
    for (i=0; i<PWM_CHANNELS; i++) {
        uint8_t old_brightness;

        /* fade channel if not already at target brightness, set flag if target reached */
        if (global_pwm.channels[i].brightness != global_pwm.channels[i].target_brightness) {
            /* safe brightness, for later compare with calculated value */
            old_brightness = global_pwm.channels[i].brightness;

            /* increase brightness */
            if (global_pwm.channels[i].brightness < global_pwm.channels[i].target_brightness) {
                /* calculate new brightness value, high byte is brightness, low byte is remainder */
                global_pwm.channels[i].brightness_and_remainder += global_pwm.channels[i].speed;

                /* if new brightness is lower than before or brightness is higher than the target,
                 * just set the target brightness and reset the remainder, since we addedd too much */
                if (global_pwm.channels[i].brightness < old_brightness || global_pwm.channels[i].brightness > global_pwm.channels[i].target_brightness) {
                    global_pwm.channels[i].brightness = global_pwm.channels[i].target_brightness;
                    global_pwm.channels[i].remainder = 0;
                }

            /* or decrease brightness */
            } else if (global_pwm.channels[i].brightness > global_pwm.channels[i].target_brightness) {
                /* calculate new brightness value, high byte is brightness, low byte is remainder */
                global_pwm.channels[i].brightness_and_remainder -= global_pwm.channels[i].speed;

                /* if new brightness is higher than before or brightness is lower than the target, just set the target brightness */
                if (global_pwm.channels[i].brightness > old_brightness || global_pwm.channels[i].brightness < global_pwm.channels[i].target_brightness) {
                    global_pwm.channels[i].brightness = global_pwm.channels[i].target_brightness;
                    global_pwm.channels[i].remainder = 0;
                }
            }

            /* if target brightness has been reached, set flag */
            if (global_pwm.channels[i].brightness == global_pwm.channels[i].target_brightness) {
                global_pwm.channels[i].flags.target_reached = 1;
            }
        }
    }
} /*}}}*/

/** prepare next timeslot */
static inline void prepare_next_timeslot(void)
/*{{{*/ {
    /* check if this is the last interrupt */
    if (pwm.index >= pwm.count) {
        /* select first timeslot and trigger timeslot rebuild */
        pwm.index = 0;
        global.flags.last_pulse = 1;
        OCR1B = 65000;
        update_pwm_timeslots();
    } else {
        /* load new top and bitmask */
        OCR1B = pwm.slots[pwm.index].top;
        pwm.next_bitmask = pwm.slots[pwm.index].mask;
        /* select next timeslot */
        pwm.index++;
    }

    /* clear compare interrupts which might have in the meantime happened */
    //TIFR |= _BV(OCF1B);
} /*}}}*/

/** interrupts*/

/** timer1 overflow (=output compare a) interrupt */
ISR(SIG_OUTPUT_COMPARE1A)
/*{{{*/ {
    if( pwm.index == 0 ){
        /* signal new cycle to main procedure */
#ifdef FAST_PWM
        //we now have a 288Hz PWM.
        //We half the new_cycle frequency to keep
        //compatibility with scripts
        static uint8_t c = 0;
        if( c++ == 1 ){
            global.flags.new_cycle = 1;
            c = 0;
        }
#else
        global.flags.new_cycle = 1;
#endif

        global.flags.timebase = 1;
#if LED_PORT_INVERT
        LED_PORT &= ~0x07;
        LED_PORT |= pwm.initial_bitmask & 0x07;
#else
        LED_PORT &= ~0x07;
        LED_PORT |= (~pwm.initial_bitmask)&0x07;
#endif
    }else{
#if LED_PORT_INVERT
        LED_PORT |= pwm.next_bitmask & 0x07;
#else
        LED_PORT &= ~(pwm.next_bitmask & 0x07);
#endif
    /* output values */
    }
    /* if next timeslot would happen too fast or has already happened, just spinlock */
    while( pwm.index < pwm.count && TCNT1 + 128 > pwm.slots[pwm.index].top)
    {
        /* spin until timer interrupt is near enough */
        while (pwm.slots[pwm.index].top > TCNT1);
        /* output value */
#if LED_PORT_INVERT
        LED_PORT |= pwm.slots[pwm.index].mask & 0x07;
#else
        LED_PORT &= ~(pwm.slots[pwm.index].mask & 0x07);
#endif
        pwm.index++;
    }
    /* prepare the next timeslot */
    prepare_next_timeslot();
} /*}}}*/

/** timer1 output compare b interrupt */
ISR(TIMER1_COMPB_vect, ISR_ALIASOF(TIMER1_COMPA_vect));
