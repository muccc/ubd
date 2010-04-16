/************************************************************************/
/*                                                                      */
/*                      RC5 Remote Receiver                             */
/*                                                                      */
/*              Author: Peter Dannegger                                 */
/*                      danni@specs.de                                  */
/*                                                                      */
/************************************************************************/
#include "config.h"
#include "rc5.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include "common.h"
#include "fnordlicht.h"

#define REPEAT_INIT 10
#define REPEAT_CONT 1

#define RC5TIME 	1.778e-3		// 1.778msec
#define PULSE_MIN	(uint8_t)(XTAL / 512 * RC5TIME * 0.4 + 0.5)
#define PULSE_1_2	(uint8_t)(XTAL / 512 * RC5TIME * 0.8 + 0.5)
#define PULSE_MAX	(uint8_t)(XTAL / 512 * RC5TIME * 1.2 + 0.5)

#if RC5_DECODER

uint8_t	    rc5_bit;				// bit value
uint8_t	    rc5_time;				// count bit time
uint16_t    rc5_tmp;				// shift bits in
uint16_t    rc5_data;				// store result

volatile uint8_t packetbase = 0;
ISR(TIMER0_OVF_vect , ISR_NOBLOCK)
{
//  sei();																	//don't block the PWM interrupt
  uint16_t tmp = rc5_tmp;				// for faster access
  uint8_t xrc5_in = 0;
  uint8_t xrc5 = 0;

  if( global.config == 21){
    xrc5_in = xRC5C1_IN;
    xrc5 = xRC5C1;
  }else if( global.config == 30 ){
    xrc5_in = xRC5C2_IN;
    xrc5 = xRC5C2;
  }

  TCNT0 = -2;					// 2 * 256 = 512 cycle

  if( ++rc5_time > PULSE_MAX ){			// count pulse time
    if( !(tmp & 0x4000) && tmp & 0x2000 )	// only if 14 bits received
      rc5_data = tmp;
    tmp = 0;
  }

  if( (rc5_bit ^ xrc5_in) & 1<<xrc5 ){		// change detect
    rc5_bit = ~rc5_bit;				// 0x00 -> 0xFF -> 0x00

    if( rc5_time < PULSE_MIN )			// to short
      tmp = 0;

    if( !tmp || rc5_time > PULSE_1_2 ){		// start or long pulse time
      if( !(tmp & 0x4000) )			// not to many bits
        tmp <<= 1;				// shift
      if( !(rc5_bit & 1<<xrc5) )		// inverted bit
        tmp |= 1;				// insert new bit
      rc5_time = 0;				// count next pulse time
    }
  }

  rc5_tmp = tmp;

/*  if(++rfm12base == 32){        //~1khz at 16mhz
    rfm12base = 0;
    global.flags.rfm12base = 0;
  }
*/
    packetbase++;
}

uint8_t rc5_checkRC5(uint16_t code)
{
	uint8_t toggle = ( code >> 11 & 1);
	static uint8_t repeats;
	static uint8_t toggle_old= 2;
	if(!code)
		return 0;
	repeats++;
	if(toggle != toggle_old){
		repeats = 0;
		toggle_old = toggle;
		return 1;
	}else if(repeats == REPEAT_INIT){
		return 1;
	}else if(repeats == REPEAT_INIT+REPEAT_CONT){
		repeats = REPEAT_INIT;
		return 1;
	}
	return 0;
}

void rc5_init(void)
{

#if defined(__AVR_ATmega324P__) ||  defined(__AVR_ATmega644P__)
    TCCR0B = 1<<CS02;           //divide by 256
    TIMSK0 |= 1<<TOIE0;         //enable timer interrupt
#else
    TCCR0 = 1<<CS02;            //divide by 256
//    TIMSK0 |= 1<<TOIE0;           //enable timer interrupt
    TIMSK |= 1<<TOIE0;          //enable timer interrupt
#endif
    if( global.config == 21){
        xRC5C1_PORT |= (1<<xRC5C1);
    }else if( global.config == 30 ){
        xRC5C2_PORT |= (1<<xRC5C2);
    }
}

#endif
