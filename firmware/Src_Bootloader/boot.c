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

/*! \file boot.c
    \brief Bootloader
    
    The bootloader is the reset entry point of the controller.
    It "speaks" just enough of the PHC protocol to allow flashing
    the main application, if instructed so.
    Normally, it would start the application after a 4 sec timeout.
    It is heavily optimized for code size.
*/

#define __FILENUM__ 15 // every file needs to have a unique (8bit) ID

// on AVRs other than Mega8 we are *really* tight with space
#if (defined(__AVR) && !defined(__AVR_ATmega8__))
#define SAVE_CODESPACE // no bells and whistles
#endif

#include <stdint.h>
#include <util/crc16.h>
#include "boot.h"
#include "hal.h"

// pick the right "mini driver" to resolve hardware dependencies
#include "uart_platform.h"

// ****** Module definitions ******
// base address and platform are now defined via makefile parameter
//#define HARDWARE_ID EM_UEM;


static volatile uint8_t timer_ticks; // system time, incremented by timer ISR

static uint8_t rcv_buf[256]; // receive buffer, large enough for uint8_t index
static volatile uint8_t rcv_fill; // write index to receive buffer

static uint8_t send_buf[256]; // send buffer, large enough for uint8_t index

// to debug, enable from the beginning
static uint8_t enabled; // set when the bootloader received the magic command

//static uint8_t leds; // debug!!


// receive interrupt handler
UART_RX_ISR
{
    if (hal_uart_has_errors()) // framing, overrun, parity error?
	{
		hal_uart_clear_errors(); // clear error flags (and interrupt)
    }	
	
    if (hal_uart_has_data())
    {
        
        if (hal_timeout2_test()) // timeout has happened?
        {
            rcv_fill = 0; // non-continuous: start new packet
        }
        hal_timeout_respawn();

	    rcv_buf[rcv_fill] = hal_uart_read_data();
	    rcv_fill++;
    }
}

// timer tick interrupt handler
TIMER_TICK_ISR
{
    hal_timer_clear_irq();

	timer_ticks++;
}

/****************** private functions ******************/


// complete CRC across a buffer
static uint16_t crc_buf(const uint8_t* buf, uint8_t n)
{
    uint16_t crc = 0xFFFF; // start value

    while(n--)
    {
		crc = _crc_ccitt_update(crc, *buf++);
    }
    
    return crc ^ 0xFFFF; // inverted output
}

static void wait_for_packet(void)
{
    for(;;) 
    {
        uint8_t packetsize;
        cli(); // make the check below atomic, else in rare race condition we may sleep despite exit condition
        packetsize = *(volatile uint8_t*)&rcv_buf[1] & 0x7F;
        if (rcv_fill >= 2 + packetsize + 2)
        {
            sei();
            break;
        }

		if (!enabled && timer_ticks > HZ*4)
		{
            hal_reboot_app(); // exit bootloader
		}

        sei();
	    hal_sleep_cpu(); // trick: the instruction behind sei gets executed atomic, too
    }

#ifdef DEBUG
    {
        uint8_t packetsize = rcv_fill;
        rcv_fill = 0; // reset before lengthy logging
        acc_printf(ACC_TYPE_LISTBOX, ACC_TYPE_STRING, (const __far void *)"packet size", 0, 1);
	    acc_printf(ACC_TYPE_LISTBOX, ACC_TYPE_UCHAR, (const __far void *)&packetsize, 0, 1);
    }
#endif
    rcv_fill = 0; // prepare next packet (race if continued reception while decoding)

	return;
}

static void send_byte(uint8_t b)
{
    uint8_t oldlevel = rcv_fill;
    uint8_t start = timer_ticks;
    hal_uart_send_data(b);

    for(;;) 
    {
        cli(); // make the check below atomic, else in rare race condition we may sleep despite exit condition
        if (rcv_fill != oldlevel // wait for echo
            || timer_ticks - start > 1) // timeout
        {
            sei();
            break;
        }
        sei();
	    hal_sleep_cpu(); // trick: the instruction behind sei gets executed atomic, too
    }

}


// compose a packet from the bytes in the global buffer and send it out
static void send_packet(uint8_t send_fill)
{
    uint16_t crc;
    uint8_t i;

    send_buf[1] |= send_fill - 2; // add length to the prepared toggle

    // add CRC to the buffer
    crc = crc_buf(send_buf, send_fill);
    //send_buf[send_fill++] = crc & 0xFF;
    //send_buf[send_fill++] = crc >> 8;
    *((uint16_t*)(&send_buf[send_fill])) = crc; // exploit little endian
    send_fill += 2;

    while(!hal_timeout1_test()); // don't send too early, wait for turnaround

    hal_uart_rs485_enable();
#ifndef SAVE_CODESPACE
    // a nice-to-have feature, save this if codespace gets real tight
    hal_delay_us(0.5 * 1000000.0/BAUDRATE); // wait for 0.5 bits time, driver settling
#endif    
    for (i=0; i<send_fill; i++)
    {
        send_byte(send_buf[i]);
        // countermeasure against receiving our echo and overwriting rcv_buf
        rcv_fill = 0;
    }
    // now, the 1st stopbit of the last byte is sent, we still need to wait a little

    hal_delay_us(1.5 * 1000000.0/BAUDRATE); // wait for 1.5 bits time, 2nd stopbit + grace

    hal_uart_rs485_disable();

}


static void send_ack(void)
{
    // send_buf already filled by default behaviour
    send_packet(4);
}


// normal ping answer, sends status
static void send_status(void)
{
    send_buf[2] = 0x01; // ping reply
    send_buf[3] = BOOTLOADER_VERSION; // which bootloader
    send_buf[4] = HARDWARE_ID & 0xFF; // which hardware
    send_buf[5] = MCU_ID; // which CPU
    send_packet(6);
}


// short ping answer for "magic ping", indicates bootloader
static void send_bootping(void)
{
    send_buf[2] = 0x01; // ping reply
    send_packet(3);
}



/****************** public functions ******************/



#ifdef __GNUC__
__attribute__((noreturn, naked)) int main(void); // less code
#endif
int main(void)
{
    uint8_t addr;
    
    hal_sysinit();
	hal_uart_init_hardware();

    addr = hal_get_addr() | DEVICE_BASEADDR;

    for (;;)
	{
		uint8_t rcv_len;
        uint16_t rcv_crc;
        uint16_t crc;
		
		wait_for_packet();

        rcv_len = rcv_buf[1] & 0x7F; // only lower 7 bits are len

        //  rcv_crc = rcv_buf[rcv_len + 2] // receive the packet CRC
        //          + ((uint16_t)rcv_buf[rcv_len + 3] << 8);
        rcv_crc = *((uint16_t*)&rcv_buf[rcv_len + 2]); // exploit little endian

		crc = crc_buf(rcv_buf, 2 + rcv_len);
		if (addr != rcv_buf[0] || crc != rcv_crc)
        {
            continue; // not for us or bad CRC
        }

        // prepare an answer, common (default) part: send ack
        send_buf[0] = rcv_buf[0]; // address
        send_buf[1] = rcv_buf[1] & 0x80; // copy toggle
        send_buf[2] = 0x00; // ack opcode
        send_buf[3] = 0x00; // no error
     
        if (rcv_len == 3
         && rcv_buf[2] == 0x01 // Ping
         && rcv_buf[3] == 'B' // with "magic" BL for bootloader
         && rcv_buf[4] == 'L')
        {
            enabled = 1;
            send_bootping();
#ifdef DEBUG
    	    acc_printf(ACC_TYPE_LISTBOX, ACC_TYPE_STRING, (const __far void *)"entering boot mode", 0, 1);
#endif
            continue; // could save this, but somehow the code gets shorter with
        }

        if (!enabled) // initially, only the magic boot command above is acceptable
        {
            continue;
        }

        switch (rcv_buf[2])
		{

        case CMD_ACK: // echo of our own response
#ifdef DEBUG
            acc_printf(ACC_TYPE_LISTBOX, ACC_TYPE_STRING, (const __far void *)"ack echo", 0, 1);
#endif
            break;
			
		case CMD_PING: // ping
            // ToDo: filter the echo of a ping response
#ifdef DEBUG
			acc_printf(ACC_TYPE_LISTBOX, ACC_TYPE_STRING, (const __far void *)"ping", 0, 1);
#endif
            send_status();

//          hal_set_led(leds ^= 2); // debug!!

            break;

        case CMD_REBOOT:
            send_ack();
            hal_reboot_app(); // reboot into application
            break; // we won't reach here

        case CMD_EEPROM_WRITE: // EEPROM write
            {
                //uint16_t addr = (uint16_t)rcv_buf[3] + ((uint16_t)rcv_buf[4] << 8);
                uint16_t addr = *((uint16_t*)&rcv_buf[3]); // exploit little endian
                uint8_t pos = 5; // data start
                rcv_len -= 3; // reduce to data size
#ifdef DEBUG
				acc_printf(ACC_TYPE_LISTBOX, ACC_TYPE_STRING, (const __far void *)"EEPROM write", 0, 1);
				acc_printf(ACC_TYPE_LISTBOX, ACC_TYPE_USHORT, (const __far void *)&addr, 0, 1);
				acc_printf(ACC_TYPE_LISTBOX, ACC_TYPE_UCHAR, (const __far void *)&rcv_len, 0, 1);
#endif
                send_ack(); // need to ack first, writing takes so long the STM would get impatient
                            // (8.5 ms per byte, on Mega8)

                hal_eeprom_write(&rcv_buf[pos], addr, rcv_len);
            }
            break;

        case CMD_LOAD_PAGE:
            {
                uint8_t address = rcv_buf[3] * BLOCKSIZE; // which part of the flash page
#ifdef DEBUG
    			acc_printf(ACC_TYPE_LISTBOX, ACC_TYPE_STRING, (const __far void *)"page load", 0, 1);
				acc_printf(ACC_TYPE_LISTBOX, ACC_TYPE_UCHAR, (const __far void *)&address, 0, 1);
#endif

#ifndef SAVE_CODESPACE
                // feature: variable size loading, allows e.g. full page if not across STM
                hal_flash_load((uint16_t*)&rcv_buf[4], address, rcv_len-2);
#else
                // fixed blocksize saves 14 byte codespace
                hal_flash_load((uint16_t*)&rcv_buf[4], address, BLOCKSIZE);
#endif
   			    send_ack();
            }
            break;

		case CMD_FLASH_PAGE: // flash page
            {
                uint8_t page = rcv_buf[3];
#ifdef DEBUG
				acc_printf(ACC_TYPE_LISTBOX, ACC_TYPE_STRING, (const __far void *)"page write", 0, 1);
				acc_printf(ACC_TYPE_LISTBOX, ACC_TYPE_UCHAR, (const __far void *)&page, 0, 1);
#endif
   			    // far OptMe: we could start erase before sending ack, but only for RWW section
                send_ack(); // need to ack first, flashing takes so long the STM would get impatient
                            // (4.5 ms erase + 4.5 ms write, on Mega8)
                hal_flash_write(page); // erase and flash it, will halt CPU for NRWW pages
            }
            break;

        case CMD_EEPROM_READ: // EEPROM read
        case CMD_FLASH_READ: // + Flash read: both responses are same format, so we can save on code
            {
                uint8_t len = rcv_buf[3];
                //uint16_t addr = (uint16_t)rcv_buf[4] + ((uint16_t)rcv_buf[5] << 8);
                uint16_t addr = *((uint16_t*)&rcv_buf[4]); // exploit little endian
                uint8_t send_fill;
#ifdef DEBUG
				acc_printf(ACC_TYPE_LISTBOX, ACC_TYPE_STRING, 
                    (const __far void *) rcv_buf[2] == CMD_EEPROM_READ ? "EEPROM read" : "Flash read", 0, 1);
				acc_printf(ACC_TYPE_LISTBOX, ACC_TYPE_USHORT, (const __far void *)&addr, 0, 1);
				acc_printf(ACC_TYPE_LISTBOX, ACC_TYPE_UCHAR, (const __far void *)&len, 0, 1);
#endif
                // not using send_ack because of payload
                send_fill = 3;
                if (rcv_buf[2] == CMD_EEPROM_READ)
                {   // copy EEPROM to answer buffer
                    hal_eeprom_read(&send_buf[send_fill], addr, len);
                }
                else // (rcv_buf[2] == CMD_FLASH_READ)
                {   // copy flash memory to answer buffer
                    hal_flash_read(&send_buf[send_fill], addr, len);
                }
                send_fill += len;
                send_packet(send_fill);
            }
            break;

        default: // unrecognized
            continue;
        } // switch
    } // for

    //for(;;); // hint no return to the compiler
#ifndef __GNUC__
	return 0;
#endif
}
