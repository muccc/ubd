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

/*! \file hal.h
    \brief Hardware abstraction layer, stripped down bootloader version.
*/

#ifndef _HAL_H
#define _HAL_H

#include "hal_platform.h" // include a platform-specific file for possible inline functions

#ifdef DEBUG
#define ASSERT(c) { if (!(c)) { hal_assert(__FILE__, __LINE__); } }
#else
#define ASSERT(c)
#endif

void hal_sysinit(void); // general system initialization

void hal_delay_us(const double us); // busy loop for forground delay

void hal_sleep_cpu(void); // set the CPU into sleep mode

#ifdef DEBUG
void hal_assert(const char* file, uint16_t line); // dump assertion message
#endif

void hal_panic(void); // signal a fatal error

uint8_t hal_get_addr(void); // read the module address from the DIP switches

void hal_set_led(uint8_t byte); // set the feedback LEDs

void hal_timer_clear_irq(void); // clear pending interrupt, for platforms which need to

// timeout timer (for receive, and send turnaround)
void hal_timeout_respawn(void);
uint8_t hal_timeout_test(void);
void hal_timeout_reset(void);

// program flash access
void hal_flash_load(uint16_t* buf, uint8_t address, uint8_t len); // prepare a page
void hal_flash_write(uint16_t page);
void hal_flash_read(void *pointer_ram, uint16_t index_flash, uint8_t n);

// eeprom access
void hal_eeprom_read (void *pointer_ram, uint16_t index_eeprom, uint8_t n);
void hal_eeprom_write (const void *pointer_ram, uint16_t index_eeprom, uint8_t n);

void hal_reboot_app(void); // restart into application code

#endif // #ifndef _HAL_H
