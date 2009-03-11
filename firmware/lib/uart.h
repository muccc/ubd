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

/*! \file uart.h
    \brief Serial port driver interface.
*/

#ifndef _UART_H
#define _UART_H

// uses message queue for incoming packets
void uart_init(uint8_t timeout); // (timeout ends up useful for collision recovery)

// send count bytes, blocking and verified
uint8_t uart_send(const uint8_t* buf, uint8_t count);

// used for waiting for "idle line"
uint8_t uart_is_busy(void);

#endif // #ifndef _UART_H
