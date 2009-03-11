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

/*! \file random.c
    \brief A primitive pseudo random number generator (PRNG)
    
    This is a simple random number generator, used to create
    a retry timing which should differ from what the other bus
    participants are trying.
*/

#define __FILENUM__ 6 // every file needs to have a unique (8bit) ID

#include <stdint.h>
#include "random.h"

static uint16_t state;

// initialize the PRNG
void rand_seed(uint16_t seed)
{
    state = seed;
}

// add some more randomness
void rand_randomize(uint8_t rnd)
{
    state ^= rnd;
}

// return 8 bit pseudo-random
uint8_t rand(void)
{
    uint8_t retval;

    state *= 75;
    retval = state >> 8;
    state += 74;

    return retval;
}
