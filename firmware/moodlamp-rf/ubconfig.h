#ifndef __UB_CONFIG_H_
#define __UB_CONFIG_H_
#include <stdint.h>

#define UB_ENABLEBRIDGE 1
#define UB_ENABLESLAVE  1
#define UB_ENABLERF     1
#define UB_ENABLERS485  1

#define UB_MAXMULTICAST 8

#define UB_PACKET_TIMEOUT   100
#define UB_PACKET_RETRIES   5

//#define USEDEBUG
typedef uint8_t ubaddress_t;
#define UB_NODEMAX      128

#define UB_INTERVAL         500
#define UB_CLASSES          {23,0,0,0}
#define UB_INITIALNODENAME  "newnode,example.com"
//rf config
#define RF_CHANNEL  23
#define RF_PORT		PORTB
#define RF_DDR		DDRB
#define RF_PIN		PINB
#define SDI		5
#define SCK		7
#define CS		4
#define SDO		6

#define RF_IRQDDR	DDRB
#define RF_IRQPIN	PINB
#define RF_IRQPORT  PORTB
#define IRQ		2

#define RF_EICR     EICRA
#define RF_EICR_MASK    (1<<ISC21)
#define RF_EIMSK    EIMSK
#define RF_EXTINT   INT2
#define RF_SIGNAL   INT2_vect


#define RESET_PORT  PORTB
#define RESET_DDR   DDRB
#define RESET       PB3

//rs485 config
#define UB_QUERYMAX     30
#define RS485_BITRATE       115200
#define RS485_ISR_EDGE      PCINT3_vect

#define RS485_DE_PIN        PC4
#define RS485_DE_PORT       PORTC
#define RS485_DE_DDR        DDRC

#define RS485_nRE_PIN       PC5
#define RS485_nRE_PORT      PORTC
#define RS485_nRE_DDR       DDRC

#define UB_PACKETLEN        50

#define UB_ESCAPE     '\\'
#define UB_NONE         '0'
#define UB_START        '1'
#define UB_STOP         '2'
#define UB_DISCOVER     '3'
#define UB_QUERY        '4'
#define UB_BOOTLOADER   '5'

#endif

