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

/*! \file boot.h
    \brief Bootloader command definitions.
*/

#ifndef _BOOT_H
#define _BOOT_H

#define BOOTLOADER_VERSION 0x21 // this is version 2.1
// A little version history:
// v1.0 was for RS485 only, not across STM, interfering with Peha modules
// v2.0 introduces a new protocol, possible across STM and Peha-friendly
// v2.1 variable length page load (Mega8 only), bugfix for Mega8 stack position


// bootloader command definitions

// I have decided against the broadcast feature, it isn't really necessary 
//#define BROADCAST        0xFC // address in the broadcast range, hopefully not used by Peha

#define CMD_ACK          0
#define CMD_PING         1
#define CMD_REBOOT       2
#define CMD_EEPROM_WRITE 3
#define CMD_LOAD_PAGE    4
#define CMD_FLASH_PAGE   5
#define CMD_EEPROM_READ  6
#define CMD_FLASH_READ   7

#define BLOCKSIZE        16 // how many bytes to transfer in CMD_LOAD_PAGE

// CPU IDs
#define MCU_MB90495     1 // Fujitsu MB90495 / MB90495
#define MCU_AVR_MEGA8   2 // Atmel AVR Mega8
#define MCU_AVR_MEGA48  3 // Atmel AVR Mega48
#define MCU_AVR_MEGA88  4 // Atmel AVR Mega88
#define MCU_AVR_MEGA168 5 // Atmel AVR Mega168

// set MCU_ID for bootloader status packet
#if defined(CPU_MB)
#define MCU_ID      MCU_MB90495;
#elif defined(__AVR_ATmega8__)
#define MCU_ID      MCU_AVR_MEGA8;
#elif defined(__AVR_ATmega48__)
#define MCU_ID      MCU_AVR_MEGA48;
#elif defined(__AVR_ATmega88__)
#define MCU_ID      MCU_AVR_MEGA88;
#elif defined(__AVR_ATmega168__)
#define MCU_ID      MCU_AVR_MEGA168;
#else
#error "unsupported controller"
#endif



// for the status packet, every kind of hardware within a class 
//   needs a unique platform ID

// class of input modules (upper byte = 0x00)
#define EM_UEM   0x0001 // UEM PCB
#define EM_IM    0x0002 // UEM modded to IM
#define EM_EM16  0x0003 // HSM+EM16
#define EM_LUX   0x0004 // UEM modded to LUX

// class of output modules (upper byte = 0x40)
#define AM_AM8   0x4001 // HSM+AM PCB
#define AM_JRM   0x4002 // HSM+AM modded to JRM


#define DEVICE_BASEADDR (HARDWARE_ID >> 8); // upper byte is module base address


#endif // #ifndef _BOOT_H
