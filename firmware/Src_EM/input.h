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

/*! \file input.h
    \brief Interface of input module application.
*/

#ifndef _INPUT_H
#define _INPUT_H

void input_init(uint8_t addr); // assign a bus address

void input_mainloop(void); // the main loop

// interrupt context functions for incoming packet processing

// packet start
void input_cmd_start(uint8_t address, uint8_t toggle, uint8_t len); 
// during payload
void input_payload(uint8_t pos, uint8_t byte);
// packet end
void input_cmd_end(uint8_t valid, uint8_t retry);

#endif // #ifndef _INPUT_H
