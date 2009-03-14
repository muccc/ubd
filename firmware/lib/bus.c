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
#include "busuart.h"
#include "timer.h"
#include "bus.h"
#include "random.h"
#include "frame.h"
#include "stdio.h"

#define RETRY_INTERVAL (HZ/2) // interval within which we view twice the same packet as a retry

/*************** module state ***************/

// received packet info
struct frame * volatile  bus_frame;
volatile struct frame  * bus_in;

static struct frame bus_buf[2];
static uint8_t bus_current = 0;

static uint8_t bus_pos; // current payload byte count
static uint16_t bus_crc_calc; // calculated CRC

#define BUS_MTU FRAME_MAX

static enum
{
	rcv_address, // 1st byte, module address
	rcv_len, // 2nd byte, length and toggle
	rcv_payload, // within packet payload, byte count done by input_pos
	rcv_crc_lsb, // 2nd last byte, CRC LSB
	rcv_crc_msb, // last byte, CRC MSB
} bus_rcv_state; // state machine for incoming bytes

// previous packet info
uint32_t bus_last_time; // timestamp of last packet in ticks 
uint16_t bus_last_crc; // just compare the CRC, not saving the full packet


/*************** private internal functions ***************/

// complete CRC across a buffer
static uint16_t crc16_frame(const struct frame * f)
{
    uint16_t crc = 0xFFFF; // start value
    uint8_t i = 0;
    crc = _crc_ccitt_update(crc, f->len);
    for(i=0;i<f->len;i++){
		crc = _crc_ccitt_update(crc, f->data[i]);
    }
    
    return crc ^ 0xFFFF; // inverted output
}

// (re)init, start new packet (possibly interrupt context)
static void packet_init(void)
{
	bus_rcv_state = rcv_len; // reset state: 1st byte for receiver
	bus_crc_calc = 0xFFFF;
}


/*************** public API functions ***************/

void bus_init(void)
{
	packet_init(); // reset receiver state machine
    bus_buf[0].isnew = 0;
    bus_buf[1].isnew = 0;
    bus_frame = &bus_buf[1];
    bus_in = /*(struct frame *)*/&bus_buf[0];
    bus_current = 0;
//    printf("bus init\r\n");
}

uint8_t bus_receive(void)
{
   return bus_frame->isnew; 
}


// complete an outgoing packet and send it, don't retry nor block, returns >= 0 on success
uint8_t bus_send(struct frame * f, uint8_t addcrc)
{
    if(addcrc)
        f->crc = crc16_frame(f); // size up to CRC
    f->data[f->len] = f->crc & 0xFF;
    f->data[f->len+1] = (f->crc >> 8)&0xFF;            //TODO: FIESER HACK!
    //printf("calculated %x %x\r\n",f->data[f->len],f->data[f->len+1]);
    return uart_send((uint8_t *)f,f->len+3);
}


// receive a byte, generic packet parsing (interrupt context)
void bus_rcv_byte(uint8_t byte)
{
	if (bus_rcv_state < rcv_crc_lsb)
    {
		bus_crc_calc = _crc_ccitt_update(bus_crc_calc, byte); // update CRC
    }
    switch (bus_rcv_state)
	{
	case rcv_len:
        rand_randomize(timer_performance_counter());
        if(bus_in->isnew){
            packet_init();
            return;
        }
		bus_in->len = byte; // without mask bit
        bus_pos = 0;
		bus_rcv_state = rcv_payload;
        //printf("len:%u\r\n",bus_in->len);
		break;

	case rcv_payload:
		bus_in->data[bus_pos++] =  byte; // call handler function
        if(bus_pos == BUS_MTU)
            bus_pos = BUS_MTU-1;

		if (bus_pos == bus_in->len)
		{
			bus_crc_calc ^= 0xFFFF; // finalize calculated CRC
			bus_rcv_state = rcv_crc_lsb;
		}
		break;
		
	case rcv_crc_lsb:
		bus_in->crc = byte;
		bus_rcv_state = rcv_crc_msb;
		break;
	
	case rcv_crc_msb:
        {
            uint8_t valid, same;
            bus_in->crc |= (uint16_t)byte << 8;
            //printf("crc calc: %x %x\r\n",bus_crc_calc&0xFF, bus_crc_calc >> 8);
		    valid = (bus_crc_calc == bus_in->crc); 
            same = ((bus_in->crc == bus_last_crc) && (timer_ticks - bus_last_time) <= RETRY_INTERVAL); // same as last, recently
            if (valid) // if valid
            {
                bus_last_crc = bus_in->crc; // remember compare values
                bus_last_time = timer_ticks;
                bus_frame = &bus_buf[bus_current];
                bus_frame->isnew = 1;
                //printf("valid bus_frame=%u\r\n",bus_frame);
                bus_current = bus_current?0:1;
                bus_in=/*(struct frame *)*/&bus_buf[bus_current];
            }else{
                //printf("frame invalid\r\n",bus_frame);
            }
		    packet_init(); // reset state machine
        }
		break;

	default:
		ASSERT(0);
	
	}
}

// receive timeout (interrupt context)
void bus_timeout(void)
{
	packet_init();
}

