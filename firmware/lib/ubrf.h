#ifndef _UBRF_H_
#define _UBRF_H_
#include "ub.h"
#include "ubpacket.h"

//init the module
void ubrf_init(void);

//periodical routine
inline void ubrf_tick(void);

//routine for the mainloop
inline void ubrf_process(void);

//check for a packet in the buffer
UBSTATUS ubrf_getPacket(struct ubpacket_t * packet);

//send a packet
//return UB_OK when a slot is free
UBSTATUS ubrf_sendPacket(struct ubpacket_t * packet);

//is the interface free to send something?
UBSTATUS ubrf_free(void);

#endif
