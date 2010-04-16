#ifndef __RFM12CONFIG_H
#define __RFM12CONFIG_H
#include "config.h"

/* config */
#if BOARD == BOARD_RF_0_1 || BOARD == BOARD_RF_0_2
#define RF_PORT		PORTB
#define RF_DDR		DDRB
#define RF_PIN		PINB
#define SDI		5
#define SCK		7
#define CS		4
#define SDO		6
#endif

#if (BOARD == BOARD_RF_0_1 || BOARD == BOARD_RF_0_2)
/*#define RF_IRQDDR	DDRD
#define RF_IRQPIN	PIND
#define RF_IRQPORT  PORTD
#define IRQ		2

#define RF_EICR     _EICRA
#define RF_EICR_MASK    (1<<ISC01)
#define RF_EIMSK    _EIMSK
#define RF_EXTINT   INT0
#define RF_SIGNAL   INT0_vect*/
#define RF_IRQDDR	DDRB
#define RF_IRQPIN	PINB
#define RF_IRQPORT  PORTB
#define IRQ		2

#define RF_EICR     _EICRA
#define RF_EICR_MASK    (1<<ISC21)
#define RF_EIMSK    _EIMSK
#define RF_EXTINT   INT2
#define RF_SIGNAL   INT2_vect


#endif

#if BOARD == BOARD_RF_0_1
#define RESET_PORT  PORTA
#define RESET_DDR   DDRA
#define RESET       PA3
#endif

#if BOARD == BOARD_RF_0_2
#define RESET_PORT  PORTB
#define RESET_DDR   DDRB
#define RESET       PB3
#endif

//#define RF12DEBUGBIN
#endif
