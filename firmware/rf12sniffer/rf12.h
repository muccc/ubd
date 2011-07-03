/*
 *
 *  RFM12 Library
 *
 *  Developer:
 *     Benedikt K.
 *     Juergen Eckert
 *
 *  Version: 2.0.1
 *
 */



#ifndef __RFM12_H
#define __RFM12_H

/* config */
/*#define RF_PORT		PORTC
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
*/
#define RF12_DataLength	100		//RF12 Buffer length
					//max length 244

//#define RF12_UseBlockingCalls		//NEVER USE BOTH
#define RF12_UseNoneBlockingCalls	//NEVER USE BOTH

/* ------ */

#define RxBW400		1
#define RxBW340		2
#define RxBW270		3
#define RxBW200		4
#define RxBW134		5
#define RxBW67		6

#define TxBW15		0
#define TxBW30		1
#define TxBW45		2
#define TxBW60		3
#define TxBW75		4
#define TxBW90		5
#define TxBW105		6
#define TxBW120		7

#define LNA_0		0
#define LNA_6		1
#define LNA_14		2
#define LNA_20		3

#define RSSI_103	0
#define RSSI_97		1
#define RSSI_91		2
#define RSSI_85		3
#define RSSI_79		4
#define RSSI_73		5
#define RSSI_67		6
#define	RSSI_61		7

#define PWRdB_0		0
#define PWRdB_3		1
#define PWRdB_6		2
#define PWRdB_9		3
#define PWRdB_12	4
#define PWRdB_15	5
#define PWRdB_18	6
#define PWRdB_21	7

#ifndef cbi
 #define cbi(sfr, bit)     (_SFR_BYTE(sfr) &= ~_BV(bit)) 
#endif
#ifndef sbi
 #define sbi(sfr, bit)     (_SFR_BYTE(sfr) |= _BV(bit))  
#endif

#define RF12TxBDW(kfrq)	((unsigned char)(kfrq/15)-1)
#define RF12FREQ(freq)	((freq-430.0)/0.0025)	// macro for calculating frequency value out of frequency in MHz


// transfer 1 word to/from module
unsigned short rf12_trans(unsigned short wert);

// initialize module
void rf12_init(void);

// set center frequency
void rf12_setfreq(unsigned short freq);

// set baudrate
void rf12_setbaud(unsigned short baud);

// set transmission settings
void rf12_setpower(unsigned char power, unsigned char mod);

// set receiver settings
void rf12_setbandwidth(unsigned char bandwidth, unsigned char gain, unsigned char drssi);


#ifdef RF12_UseBlockingCalls
// transmit number of bytes from array
void rf12_txdata(unsigned char *data, unsigned char number);

// receive number of bytes into array
unsigned char rf12_rxdata(unsigned char *data);

// wait until FIFO ready (to transmit/read data)
void rf12_ready(void);
#endif

#ifdef RF12_UseNoneBlockingCalls

// start receiving a package
unsigned char rf12_rxstart(void);

// readout the package, if one arrived
unsigned char rf12_rxfinish(unsigned char *data);

// start transmitting a package of size size
unsigned char rf12_txstart(unsigned char *data, unsigned char size);

// check whether the package is already transmitted
unsigned char rf12_txfinished(void);

// stop all Rx and Tx operations
void rf12_allstop(void);

extern volatile unsigned char RF12_Index;
extern volatile unsigned char rf12_checkcrc;
#endif


#endif
