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

//#define RETRY_INTERVAL (HZ/2) // interval within which we view twice the same packet as a retry

/*************** module state ***************/

// received packet info
struct frame * volatile  bus_frame;
volatile struct frame  * bus_in;
static struct frame out;

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
//uint32_t bus_last_time; // timestamp of last packet in ticks 
//uint16_t bus_last_crc; // just compare the CRC, not saving the full packet
uint16_t timeout;
uint8_t retry;
uint8_t txstate;
struct frame * txframe;

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
static void bus_packet_init(void)
{
    bus_rcv_state = rcv_len; // reset state: 1st byte for receiver
    bus_crc_calc = 0xFFFF;
}


/*************** public API functions ***************/

void bus_init(void)
{
    bus_packet_init(); // reset receiver state machine
    bus_buf[0].isnew = 0;
    bus_buf[1].isnew = 0;
    bus_frame = &bus_buf[1];
    bus_in = /*(struct frame *)*/&bus_buf[0];
    bus_current = 0;
//    printf("bus init\r\n");
    DDRB |= (1<<PB1);
    PORTB |= (1<<PB1);
}

//ticks the bus and checks for retrys.
void bus_tick(void)
{
    //uint8_t r;
    switch(uart_txresult()){
        case UART_NULL:             //nothing happened
        break;
        case UART_OK:               //packet was transmitted
            txstate = TX_DONE;
            PORTC &= ~(1<<PC3);
            uart_txreset();
            break;
        default:                    //an error occoured
            PORTB ^= (1<<PB1);
            uart_txreset();         //don't trigger again
            //PORTC &= ~(1<<PC3);
            timeout+=rand()&0xF;        //increase backoff timer
            if(timeout >= 1000){         //check for timeout after ca. 1s.
                txstate = TX_TIMEOUT;
                break;
            }
            retry = timeout;        //schedule a retry
            break;
    };
    if(retry && --retry == 0){              //retry counter check
        uart_send((uint8_t *)txframe,txframe->len+3);
    }
}

//check if an uprocessed frame is available
uint8_t bus_receive(void)
{
   return bus_frame->isnew; 
}

//check if a packet was transmitted or if a timeout occoured(bus broken?)
uint8_t bus_done(void)
{
    return txstate;
}

// setup an outgoing packet. Starts the transmitting isr and resets the tx statmachine
void bus_send(struct frame * f, uint8_t addcrc)
{
    if(addcrc)
        f->crc = crc16_frame(f); // size up to CRC
    f->data[f->len] = f->crc & 0xFF;
    f->data[f->len+1] = (f->crc >> 8)&0xFF;            //XXX: FIESER HACK! sollte in uart_send passieren
    while(uart_is_busy());
    memcpy(&out,f,sizeof(struct frame));
    txframe = &out;
    PORTC |= (1<<PC3);
    uart_randomize(rand());
    timeout = 0;
    txstate = TX_NULL;
    uart_send((uint8_t *)txframe,txframe->len+3);
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
            bus_packet_init();
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
            uint8_t valid;//, same;
            bus_in->crc |= (uint16_t)byte << 8;
            //printf("crc calc: %x %x\r\n",bus_crc_calc&0xFF, bus_crc_calc >> 8);
            valid = (bus_crc_calc == bus_in->crc); 
            //same = ((bus_in->crc == bus_last_crc) && (timer_ticks - bus_last_time) <= RETRY_INTERVAL); // same as last, recently
            if (valid) // if valid
            {
                //bus_last_crc = bus_in->crc; // remember compare values
                //bus_last_time = timer_ticks;
                bus_frame = &bus_buf[bus_current];
                bus_frame->isnew = 1;
                //printf("valid bus_frame=%u\r\n",bus_frame);
                bus_current = bus_current?0:1;
                bus_in=/*(struct frame *)*/&bus_buf[bus_current];
            }else{
                //printf("frame invalid\r\n",bus_frame);
            }
            bus_packet_init(); // reset state machine
        }
        break;

    default:
        ASSERT(0);
    
    }
}

// receive timeout (interrupt context)
void bus_timeout(void)
{
    bus_packet_init();      //we have to abort the current packet
}

