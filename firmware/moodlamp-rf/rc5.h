#ifndef __RC5_H_
#define __RC5_H_

#include <stdint.h>

#define FCPU F_CPU
#define XTAL FCPU

extern uint16_t	rc5_data;				// store result
extern volatile uint8_t packetbase;

//See http://www.armory.com/~spcecdt/remote/RC5codes.html
#define RC5_ADR_TV		0
#define RC5_ADR_VCR		5		//Preset for VCR1 on URC22B-14
#define RC5_ADR_RECV	17
#define RC5_ADR_TAPE	18

//This list works for the standard VCR1 setting of my universal remote (URC22B-14 sold by Conrad in Germany)
//They work also on the VCR2 setting of the IM-1313 also sold by Conrad
#define RC5_1		1
#define RC5_2		2
#define RC5_3		3
#define RC5_4		4
#define RC5_5		5
#define RC5_6		6
#define RC5_7		7
#define RC5_8		8
#define RC5_9		9
#define RC5_10		0
#define RC5_11		10
#define RC5_12		35
#define RC5_MEMO	15
#define RC5_CONTRAST_UP		63
#define RC5_CONTRAST_DOWN	43
#define RC5_BRIGHTNESS_UP	18
#define RC5_BRIGHTNESS_DOWN	19
#define RC5_COLOR_UP		20
#define RC5_COLOR_DOWN		21
#define RC5_CHAN_UP			32
#define RC5_CHAN_DOWN		33
#define RC5_SKIP_FORWARD	40
#define RC5_SKIP_BACKWARD	34
#define RC5_FORWARD			52
#define RC5_BACKWARD		50
#define RC5_PLAY			53
#define RC5_VOL_UP			16
#define RC5_VOL_DOWN		17
#define RC5_RED				14
#define RC5_GREEN			47
#define RC5_YELLOW			59
#define RC5_BLUE			60
#define RC5_RECORD			55
#define RC5_PAUSE			48
#define RC5_STOP			54
#define RC5_POWER			12
#define RC5_PRESETS			49
#define RC5_MUTE			13
#define RC5_A               44
#define RC5_B               46
#define RC5_C               37

uint8_t rc5_checkRC5(uint16_t code);
void rc5_init(void);
#endif
