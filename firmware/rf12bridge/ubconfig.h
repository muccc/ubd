#ifndef __UB_CONFIG_H_
#define __UB_CONFIG_H_
#include <stdint.h>

#define TXLED
#define TXLED_PORT  A
#define TXLED_PIN   4

#define RXLED
#define RXLED_PORT  A
#define RXLED_PIN   5

#define USEDEBUG

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

#define UB_PACKETLEN        128

#define UB_ESCAPE     '\\'
#define UB_NONE         '0'
#define UB_START        '1'
#define UB_STOP         '2'
#define UB_DISCOVER     '3'
#define UB_QUERY        '4'
#define UB_BOOTLOADER   '5'

#endif

