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

/*! \file errlog.c
    \brief Log errors in some way
    
    This writes a C file ID and the line number of a triggered assertion
    to a circular EEPROM section, for later readout.
*/

#define __FILENUM__ 1 // every file needs to have a unique (8bit) ID

#include <stdint.h>
#include "errlog.h"
#include "hal.h"
#include <avr/eeprom.h> // for EEPROM access
#include <avr/interrupt.h> // for interrupt disable

#define DEPTH ((256-1)/3) // use up to 256 bytes for log (max for uint8_t)

void errlog(uint8_t filenum, uint16_t linenum)
{
    uint8_t idx;
    uint8_t pos;
    uint8_t sreg;

	sreg = SREG;
    cli(); // make the writing atomic
    idx = eeprom_read_byte(0); // next write position
    if (idx == 0xFF) // virgin
    {
        idx = 0;
    }

    if (idx > DEPTH)
    {
        idx = 0;
        
        //SREG = sreg; return; // debug hack: dont' overwrite
    }

    pos = 1 + idx * 3; // 3 byte entries, skip index byte
    idx++; // new next position

    eeprom_write_byte((void*)0, idx);
    // ugly casting because eeprom_write_byte wants a pointer and gcc is picky
    eeprom_write_byte((void*)((uint16_t)pos++), filenum);
    eeprom_write_byte((void*)((uint16_t)pos++), linenum >> 8); // highbyte
    eeprom_write_byte((void*)((uint16_t)pos), linenum & 0xFF); // lowbyte
    SREG = sreg; // sei();
}
