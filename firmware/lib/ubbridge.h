#ifndef _UBBRIDGE_H_
#define _UBBRIDGE_H_
#include "ubpacket.h"
#include "ub.h"

void ubbridge_init(void);
inline void ubbridge_tick(void);
inline void ubbridge_process(void);
inline UBSTATUS ubbridge_sendPacket(struct ubpacket_t * packet);
inline uint8_t ubbridge_getPacket(struct ubpacket_t * packet);
void ubbridge_forward(struct ubpacket_t * packet);
void ubbridge_done(void);
void ubbridge_abort(void);
#endif
