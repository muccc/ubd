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


#ifndef _BUS_H_
#define _BUS_H_
#include "frame.h"

void bus_init(void);

// compose an outgoing packet and send it, return error when failed
uint8_t bus_send(struct frame * f,  uint8_t addcrc);

// receive a byte, generic packet parsing (interrupt context)
void bus_rcv_byte(uint8_t byte);

// receive timeout (interrupt context)
void bus_timeout(void);

extern volatile struct frame * bus_frame;
#endif // #ifndef _PHC_PROTOCOL_H
