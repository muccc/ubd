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

/*! \file timer.h
    \brief Timer services interface.
*/

#ifndef _TIMER_H
#define _TIMER_H

#include "msg.h"
#include "timer_platform.h" // hardware specific part


extern uint32_t timer_ticks; // directly export the tick count

void timer_init(void (*callback)(void), uint8_t initvalue);

// delayed message sending
void timer_msg(uint32_t ticks);

// high resolution timer
uint16_t timer_performance_counter(void);

#endif // #ifndef _TIMER_H
