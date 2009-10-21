#ifndef _UBSLAVE_H_
#define _UBSLAVE_H_
#include "ub.h"
#include "ubpacket.h"

inline void ubslave_init(void);
inline void ubslave_process(void);
inline void ubslave_tick(void);
inline UBSTATUS ubslave_sendPacket(struct ubpacket_t * packet);
inline uint8_t ubslave_getPacket(struct ubpacket_t * packet);
#endif
