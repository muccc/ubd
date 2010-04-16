#ifndef _UBMASTER_H_
#define _UBMASTER_H_
#include "ubpacket.h"
#include "ub.h"

void ubmaster_init(void);
inline void ubmaster_tick(void);
inline void ubmaster_process(void);
inline UBSTATUS ubmaster_sendPacket(struct ubpacket_t * packet);
inline uint8_t ubmaster_getPacket(struct ubpacket_t * packet);
void ubmaster_forward(struct ubpacket_t * packet);
void ubmaster_done(void);
void ubmaster_abort(void);
#endif
