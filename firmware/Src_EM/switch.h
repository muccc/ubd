/***************************************************************************
 *
 * OpenHC:                          ___                 _  _  ___
 *  Open source                    / _ \ _ __  ___ _ _ | || |/ __|
 *  Home                          | (_) | '_ \/ -_) ' \| __ | (__ 
 *  Control                        \___/| .__/\___|_||_|_||_|\___|
 * http://openhc.sourceforge.net/       |_| 
 *
 * Copyright (C) 2005 by Joerg Hohensohn
 *
 * All files in this archive are subject to the GNU General Public License.
 * See http://www.gnu.org/licenses/gpl-3.0.txt for full license agreement.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ****************************************************************************/ 

/*! \file switch.h
    \brief Interface of input module application helper.
*/

#ifndef _SWITCH_H
#define _SWITCH_H

#include "msg.h"

#define NUM_INPUTS CFG_INPUT // # of available inputs (8 or 16) as set in makefile

// determine which data type is needed for the above #
#if (NUM_INPUTS <= 8)
typedef uint8_t switch_t;
#elif (NUM_INPUTS <= 16)
typedef uint16_t switch_t;
#else
#error Unsupported input width!
#endif

void switch_init(void);

void switch_set_enable(switch_t* p_enables);

switch_t switch_getcurrent(void); // return current input state

void switch_setcurrent(switch_t lines); // set initial input state

uint8_t switch_getevents(uint8_t* dest, uint8_t bufsize, uint8_t pos);

void switch_tick(void); // poll the switches

void switch_ack(void); // roundtrip ack

void switch_nack(void); // negative roundtrip ack

#endif // #ifndef _SWITCH_H
