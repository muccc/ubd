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

/*! \file phc.c
    \brief A small PHC stack, general PHC functions
    
    This is common code to breakdown the generic PHC protocol part.
*/

#define __FILENUM__ 5 // every file needs to have a unique (8bit) ID

#include <stdint.h>
#include <util/crc16.h>
#include "hal.h"
#include "uart.h"
#include "timer.h"
#include "phc.h"
#include "random.h"
#ifdef DEBUG
#include "monitor.h" // for debug dump  ToDo: encapsulate into HAL
#endif

#define RETRY_INTERVAL (HZ/2) // interval within which we view twice the same packet as a retry

/*************** module state ***************/

static void (*pch_fn_start)(uint8_t address, uint8_t toggle, uint8_t len);
static void (*phc_fn_payload)(uint8_t pos, uint8_t byte);
static void (*phc_fn_end)(uint8_t valid, uint8_t retry);

// received packet info
static uint8_t phc_addr; // address byte
static uint8_t phc_len; // total payload byte count
static uint8_t phc_pos; // current payload byte count
static uint16_t phc_crc_rcv; // received CRC at packet end
static uint16_t phc_crc_calc; // calculated CRC
static enum
{
	rcv_address, // 1st byte, module address
	rcv_len, // 2nd byte, length and toggle
	rcv_payload, // within packet payload, byte count done by input_pos
	rcv_crc_lsb, // 2nd last byte, CRC LSB
	rcv_crc_msb, // last byte, CRC MSB
} phc_rcv_state; // state machine for incoming bytes

// previous packet info
uint32_t phc_last_time; // timestamp of last packet in ticks 
uint16_t phc_last_crc; // just compare the CRC, not saving the full packet


/*************** private internal functions ***************/

// complete CRC across a buffer
static uint16_t crc16_buf(const uint8_t* buf, uint8_t n)
{
    uint16_t crc = 0xFFFF; // start value

    while(n--)
    {
		crc = _crc_ccitt_update(crc, *buf++);
    }
    
    return crc ^ 0xFFFF; // inverted output
}

// (re)init, start new packet (possibly interrupt context)
static void packet_init(void)
{
	phc_rcv_state = rcv_address; // reset state: 1st byte for receiver
	phc_crc_calc = 0xFFFF;
}


/*************** public API functions ***************/

void phc_init(
    void (*cmd_start)(uint8_t address, uint8_t toggle, uint8_t len),
    void (*cmd_payload)(uint8_t pos, uint8_t byte),
    void (*cmd_end)(uint8_t valid, uint8_t retry))
{
    pch_fn_start = cmd_start;
    phc_fn_payload = cmd_payload;
    phc_fn_end = cmd_end;
    
	packet_init(); // reset receiver state machine
}

#ifdef DEBUG
// debug function
void phc_packet_dump(const uint8_t* packet, uint8_t size)
{
	const static char hex[] = "0123456789ABCDEF";
	static char log_buf[3*33+1]; // # of bytes plus termination
	char* run = log_buf;

	if (size*3+1 > sizeof(log_buf))
	{
		size = sizeof(log_buf) / 3;
	}

	while (size--)
	{	// convert to hex string
		*run++ = hex[*packet / 16];
		*run++ = hex[*packet++ % 16];
		*run++ = ' ';
	}	
	*run++ = '\0';
	acc_printf(ACC_TYPE_LISTBOX, ACC_TYPE_STRING, (const __far void*)log_buf, 0, 1);
}
#endif


// complete an outgoing packet and send it, don't retry nor block, returns >= 0 on success
uint8_t phc_send(uint8_t addr, uint8_t* packet, uint8_t size, uint8_t toggle)
{
	uint16_t crc;

	packet[0] = addr;
	packet[1] = size | ((toggle) ? 0x80 : 0x00);

	size += 2; // including address and length field

    crc = crc16_buf(packet, size); // size up to CRC

    packet[size]   = crc & 0xFF;
	packet[size+1] =  crc >> 8;

	size += 2; // size including CRC
    
    return uart_send(packet, size);
}


// receive a byte, generic packet parsing (interrupt context)
void phc_rcv_byte(uint8_t byte)
{
	if (phc_rcv_state < rcv_crc_lsb)
    {
		phc_crc_calc = _crc_ccitt_update(phc_crc_calc, byte); // update CRC
    }
    
    switch (phc_rcv_state)
	{
	case rcv_address:
		phc_addr = byte;
		phc_rcv_state = rcv_len;
        // use external event to get more randomness
        rand_randomize(timer_performance_counter());
		break;

	case rcv_len:
		phc_len = byte & 0x7F; // without mask bit
		pch_fn_start(phc_addr, byte >> 7, phc_len); // call start handler
		if (phc_len)
		{
			phc_pos = 0;
			phc_rcv_state = rcv_payload;
		}
		else
		{
			phc_rcv_state = rcv_crc_lsb;
		}
		break;

	case rcv_payload:
		phc_fn_payload(phc_pos++, byte); // call handler function	
		if (phc_pos == phc_len)
		{
			phc_crc_calc ^= 0xFFFF; // finalize calculated CRC
			phc_rcv_state = rcv_crc_lsb;
		}
		break;
		
	case rcv_crc_lsb:
		phc_crc_rcv = byte;
		phc_rcv_state = rcv_crc_msb;
		break;
	
	case rcv_crc_msb:
        {
            uint8_t valid, same;

            phc_crc_rcv |= (uint16_t)byte << 8;
		    valid = (phc_crc_calc == phc_crc_rcv); 
            same = ((phc_crc_rcv == phc_last_crc) && (timer_ticks - phc_last_time) <= RETRY_INTERVAL); // same as last, recently
		    phc_fn_end(valid, same); // call closing handler

            if (valid) // if valid
            {
                phc_last_crc = phc_crc_rcv; // remember compare values
                phc_last_time = timer_ticks;
            }
		    packet_init(); // reset state machine
        }
		break;

	default:
		ASSERT(0);
	
	}
}

// receive timeout (interrupt context)
void phc_timeout(void)
{
	packet_init();
}

