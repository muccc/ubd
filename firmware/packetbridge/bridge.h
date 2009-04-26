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

#ifndef _BRIDGE_H
#define _BRIDGE_H

void bridge_init(uint8_t addr); // assign a bus address

void bridge_mainloop(void); // the main loop

void bridge_tick(void); // timer tick ISR

void bridge_output(void);   //check for new packets on the bus and forward them

void bridge_input(void);    //get input from the serial line and prcess it

void bridge_status(void);   //report events on the serial line
#endif // #ifndef _OUTPUT_H
