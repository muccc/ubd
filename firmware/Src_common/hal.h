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
    \brief Hardware abstraction layer.
*/

#ifndef _HAL_H
#define _HAL_H

// profiler definitions
enum
{
    PF_SLEEP,
    PF_DUMMY,    
    PF_ISRSTART_TIMER,
    PF_ISREND_TIMER,
    PF_ISRSTART_RX_EDGE,
    PF_ISREND_RX_EDGE,
    PF_ISRSTART_RX,
    PF_ISREND_RX,
    PF_ISRSTART_TX,
    PF_ISREND_TX,
    PF_ISRSTART_TX_DONE,
    PF_ISREND_TX_DONE,
    PF_ISRSTART_TX_TIMEOUT,
    PF_ISREND_TX_TIMEOUT,
    PF_ISRSTART_RX_TIMEOUT,
    PF_ISREND_RX_TIMEOUT,
    PF_ISRSTART_RX_TIMEOUT2,
    PF_ISREND_RX_TIMEOUT2,
    PF_ISRSTART_ADC,
    PF_ISREND_ADC,
};
//#define PROFILE(event) hal_profile(event) // event is one of PF_xx
#define PROFILE(event)

#include "hal_platform.h" // include a platform-specific file for possible inline functions

#ifdef DEBUG
#define ASSERT(c) { if (!(c)) { hal_assert(__FILE__, __LINE__); } } // verbose
#else
#define ASSERT(c) { if (!(c)) { hal_assert(__FILENUM__, __LINE__); } } // short
#endif


void hal_sysinit(void); // general system initialization

void hal_delay_us(const double us); // busy loop for forground delay
void hal_suspend(void); // set the CPU into sleep mode, all of the 3 below
void hal_sleep_enable(void); // prepare sleeping
void hal_sleep_cpu(void); // the actual sleep instruction
void hal_sleep_disable(void); // prevent inadvertent sleeping

#ifdef DEBUG
void hal_assert(const char* file, uint16_t line); // dump assertion message
#else
void hal_assert(uint8_t filenum, uint16_t line); // dump assertion message
#endif

void hal_panic(void); // signal a fatal error

void hal_watchdog_enable(void); // enable watchdog timer
void hal_watchdog_reset(void); // reset watchdog timer
void hal_reboot(void); // full restart, into bootloader if present

uint8_t hal_get_addr(void); // read the module address from the DIP switches

void hal_set_led(uint8_t byte); // set the feedback LEDs

#ifdef CFG_INPUT
uint16_t hal_get_switch(void); // read the input switches
#elif CFG_OUTPUT
void hal_set_output(uint8_t byte); // set the outputs
void hal_set_pwm(uint8_t byte); // pulsed PWM output to save relay power
#endif

#ifdef CFG_IR
uint16_t hal_get_capture(void); // read the timer capture
uint8_t hal_get_edge(void); // query the capture edge, returns 0=falling, 1=rising
void hal_set_edge(uint8_t edge); // set the capture edge, 0=falling, 1=rising
#endif

#ifdef CFG_ADC
uint16_t hal_read_voltage(void); // return the measured supply voltage (in mV)
void hal_start_adc(void); // start an ADC conversion

#endif


#endif // #ifndef _HAL_H
