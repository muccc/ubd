#ifndef _UBRS485SLAVE_H_
#define _UBRS485SLAVE_H_
#include <stdint.h>
#include "ub.h"
#include "ubpacket.h"

inline void rs485slave_rx(void);
inline void rs485slave_tx(void);
inline void rs485slave_edge(void);
inline void rs485slave_timer(void);
inline void rs485slave_txend(void);
void rs485slave_init(void);
void rs485slave_stop(void);
inline void rs485slave_tick(void);
inline void rs485slave_process(void);
UBSTATUS rs485slave_sendPacket(struct ubpacket_t * packet);
void rs485slave_start(uint8_t start, uint8_t * data, uint8_t len, uint8_t stop);
inline uint8_t rs485slave_getPacket(struct ubpacket_t * packet);
inline void rs485slave_setConfigured(uint8_t configured);
inline uint8_t rs485slave_getConfigured(void);
inline void rs485slave_transmit(void);
#endif
