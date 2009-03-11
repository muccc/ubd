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

/*! \file errlog.h
    \brief Interface of error logger service.
*/

#ifndef _ERRLOG_H
#define _ERRLOG_H

/* Filenum index:
    1 errnum.c
    2 hal_atmel.c
    3 hal_mb90495.c
    4 msg.c
    5 phc.c
    6 random.c
    7 timer.c
    8 uart.c
    9 vectors.c

    10 input.c
    11 main.c (input)
    12 switch.c

    13 main.c (output)
    14 output.c
    
    15 boot.c
    16 hal_atmel
    17 main.c

*/


void errlog(uint8_t filenum, uint16_t linenum);


#endif // #ifndef _ERRLOG_H

