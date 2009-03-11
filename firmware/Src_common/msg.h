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

/*! \file msg.h
    \brief Message queue interface.
*/

#ifndef _MSG_H
#define _MSG_H

// the known events
// ToDo: this can be application specific, no need to define unique values here
enum msg_id
{
	e_nop, // void message
	e_send_state, // send LED/output state, data is toggle bit
	e_send_timer, // send JRM timer state, data is toggle bit
	e_send_ping, // send ping response, data is toggle bit
	e_timer, // timer expiration
	e_event, // input switch, data is queue position
	e_switch_on, // output line change, data is bit#
	e_switch_off, // output line change, data is bit#
    e_timeaction, // output changed by timed action
    e_send_timerstate, // send state of JRM timer, data is channel (0..3)
    e_up, // lift up, data is channel #
    e_down, // roll down, data is channel #
    e_stop, // stop movement, data is channel #
    e_learned, // IR teach-in has learned a code
	e_last	
};


// message packet
struct msg
{
	enum msg_id id; // which message is this
	uint8_t data;   // content depends on ID
};


uint8_t msg_post(struct msg* message); // post a message, also allowed from interrupt context
struct msg msg_get(void); // wait for a message

#endif // #ifndef _MSG_H
