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

/*! \file phc.h
    \brief General PHC protocol definition and stack functions.
*/

#ifndef _PHC_PROTOCOL_H
#define _PHC_PROTOCOL_H

// time constants
#define ACK_TO 2 // minimum ticks waiting for response before retransmitting

// protocol values

// input events
#define INPUT_ON_GREATER_0   2
#define INPUT_OFF_LESS_1     3
#define INPUT_ON_GREATER_1   4
#define INPUT_OFF_GREATER_1  5
#define INPUT_ON_GREATER_2   6
#define INPUT_OFF            7

// output commands
#define OUTPUT_ON            2
#define OUTPUT_OFF           3
#define OUTPUT_LOCK_ON       4
#define OUTPUT_LOCK_OFF      5
#define OUTPUT_TOGGLE        6
#define OUTPUT_UNLOCK        7
#define OUTPUT_DELAY_ON      8
#define OUTPUT_DELAY_OFF     9
#define OUTPUT_TIMED_ON     10
#define OUTPUT_TIMED_OFF    11
#define OUTPUT_DELAY_TOGGLE 12
#define OUTPUT_TIMED_TOGGLE 13
#define OUTPUT_LOCK         14
#define OUTPUT_TIMED_LOCK   15
#define OUTPUT_TIME_ADD     16
#define OUTPUT_TIME_SET     17
#define OUTPUT_TIME_CANCEL  18

// output events (notifications)
#define OUTPUT_FB_ON         2
#define OUTPUT_FB_OFF        3
#define OUTPUT_FB_TIMER   0xFD

// JRM commands
#define JRM_STOP               2
#define JRM_TOGGLE_LIFT_STOP   3
#define JRM_TOGGLE_LOWER_STOP  4
#define JRM_LIFT               5
#define JRM_LOWER              6
#define JRM_FLIP_UP            7
#define JRM_FLIP_DOWN          8
#define JRM_PRIO_LOCK          9
#define JRM_PRIO_UNLOCK       10
#define JRM_LEARN_ON          11
#define JRM_LEARN_OFF         12
#define JRM_PRIO_SET          13
#define JRM_PRIO_CLEAR        14
#define JRM_SENSOR_LIFT       15
#define JRM_SENSOR_LIFT_FLIP  16
#define JRM_SENSOR_LOWER      17
#define JRM_SENSOR_LOWER_FLIP 18

#define JRM_TIMER_DELAY_ON    19
#define JRM_TIMER_DELAY_OFF   20
#define JRM_TIMER_ON_OFF      21
#define JRM_TIMER_CANCEL      22

// JRM events (notifications)
#define JRM_FB_LIFT_ON         2
#define JRM_FB_LOWER_ON        3
#define JRM_FB_LIFT_OFF        4
#define JRM_FB_LOWER_OFF       5
#define JRM_FB_TIMER_ON        6
#define JRM_FB_TIMER_CANCEL    7
#define JRM_FB_TIMER_OFF       8



// init with 3 callbacks, for the command phases
void phc_init(
    void (*cmd_start)(uint8_t address, uint8_t toggle, uint8_t len),
    void (*cmd_payload)(uint8_t pos, uint8_t byte),
    void (*cmd_end)(uint8_t valid, uint8_t retry));

#ifdef DEBUG
// debug function
void phc_packet_dump(const uint8_t* packet, uint8_t size);
#endif

// compose an outgoing packet and send it, return error when failed
uint8_t phc_send(uint8_t addr, uint8_t* payload, uint8_t size, uint8_t toggle);

// receive a byte, generic packet parsing (interrupt context)
void phc_rcv_byte(uint8_t byte);

// receive timeout (interrupt context)
void phc_timeout(void);

#endif // #ifndef _PHC_PROTOCOL_H
