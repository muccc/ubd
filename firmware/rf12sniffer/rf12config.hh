#ifndef __RFM12CONFIG_H
#define __RFM12CONFIG_H

/* config */
#define RF_PORT		PORTC
#define RF_DDR		DDRC
#define RF_PIN		PINC
#define SDI		0
#define SCK		1
#define CS		2
#define SDO		3

#define RF_IRQDDR	DDRD
#define RF_IRQPIN	PIND
#define RF_IRQPORT  PORTD
#define IRQ		2		//only used if async mode. Had to be INT0
					//or INT1/2 when change something in rf12.c
#endif
