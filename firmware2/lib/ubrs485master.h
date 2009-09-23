#ifndef _UBRS485MASTER_H_
#define _UBRS485MASTER_H_
#include "ub.h"

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
uint8_t rs485master_getMessage(uint8_t * buffer);

//send a packet
//return UB_OK when a slot is free
UBSTATUS rs485master_send(uint8_t * data, uint8_t len);


//internal functions

//start sending a frame.
//if data equals NULL no body will be send
//if stop equals 0 no stop escape will be sent
/*static*/ void rs485master_start(uint8_t start, uint8_t * data,
                                uint8_t len, uint8_t stop);

//check if a slot contains data and transmit
/*static*/ void rs485master_runslot(void);

#endif
