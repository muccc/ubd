#ifndef _UBRS485MASTER_H_
#define _UBRS485MASTER_H_
#include "ub.h"
#include "ubpacket.h"

//interrupts from the timer or the uart
inline void rs485master_rx(void);
inline void rs485master_tx(void);
inline void rs485master_edge(void);
inline void rs485master_timer(void);
inline void rs485master_txend(void);

//init the module
void rs485master_init(void);

//periodical routine
inline void rs485master_tick(void);

//routine for the mainloop
inline void rs485master_process(void);

//check for a packet in the buffer
int16_t rs485master_getPacket(struct ubpacket_t * packet);

//send a packet
//return UB_OK when a slot is free
UBSTATUS rs485master_sendPacket(struct ubpacket_t * packet);

//is the interface free to send something?
UBSTATUS rs485master_free(void);

//querry a module in a given interval in ms
UBSTATUS rs485master_setQueryInterval(uint8_t adr, uint16_t interval);
//internal functions

//start sending a frame.
//if data equals NULL no body will be send
//if stop equals 0 no stop escape will be sent
/*static*/ void rs485master_start(uint8_t start, uint8_t * data,
                                uint8_t len, uint8_t stop);

//check if a slot contains data and transmit
/*static*/ void rs485master_runslot(void);

#endif
