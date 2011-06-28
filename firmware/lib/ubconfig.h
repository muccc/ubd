#ifndef _UBCONFIG_H_
#define _UBCONFIG_H_
#include <stdint.h>

#include "ubclasses.h"
#include "deviceconfig.h"
#include "busconfig.h"

#define UB_MAXMULTICAST 8

#define UB_INITIALNODENAME  "newnode,example.com"
typedef uint8_t ubaddress_t;
#define UB_NODEMAX      128

#define UB_PACKET_TIMEOUT   100
#define UB_RF_TIMEOUT       100
#define UB_RS485_TIMEOUT   (UB_INTERVAL + 200)
#define UB_PACKET_RETRIES   6

#define UB_PACKETLEN        50

#define UB_ESCAPE     '\\'
#define UB_NONE         '0'
#define UB_START        '1'
#define UB_STOP         '2'
#define UB_DISCOVER     '3'
#define UB_QUERY        '4'
#define UB_BOOTLOADER   '5'

#ifdef BOARD_DEVBOARD

#define TXLED
#define TXLED_PORT  C
#define TXLED_PIN   5

//rf config
#define RF_PORT		PORTB
#define RF_DDR		DDRB
#define RF_PIN		PINB
#define SDI		5
#define SCK		7
#define CS		1
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
#define RESET       PB0

//rs485 config
#define RS485_BITRATE       115200
#define RS485_ISR_EDGE      PCINT2_vect
#define RS485_MASK_EDGE     PCMSK2
#define RS485_PIN_EDGE      PCINT20
#define RS485_PCIF_EDGE     PCIF2
#define RS485_PCIE_EDGE     PCIE2

#define RS485_DE_PIN        PC2
#define RS485_DE_PORT       PORTC
#define RS485_DE_DDR        DDRC

#define RS485_nRE_PIN       PC3
#define RS485_nRE_PORT      PORTC
#define RS485_nRE_DDR       DDRC

#endif //devboard

#ifdef BOARD_MOODLAMP
//rf config
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
#define RS485_MASK_EDGE     PCMSK3
#define RS485_PIN_EDGE      PCINT28
#define RS485_PCIF_EDGE     PCIF3
#define RS485_PCIE_EDGE     PCIE3




#define RS485_DE_PIN        PC4
#define RS485_DE_PORT       PORTC
#define RS485_DE_DDR        DDRC

#define RS485_nRE_PIN       PC5
#define RS485_nRE_PORT      PORTC
#define RS485_nRE_DDR       DDRC
#endif //MOODLAMP

#endif
