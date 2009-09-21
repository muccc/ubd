#ifndef _UBRS485SLAVE_H_
#define _UBRS485SLAVE_H_
#include <stdint.h>
#include "ub.h"

inline void rs485slave_rx(void);
inline void rs485slave_tx(void);
inline void rs485slave_edge(void);
inline void rs485slave_timer(void);
inline void rs485slave_txend(void);
void rs485slave_init(void);
inline void rs485slave_tick(void);
inline void rs485slave_process(void);
UBSTATUS rs485client_send(uint8_t * data, uint8_t len);
void rs485slave_start(uint8_t start, uint8_t * data, uint8_t len, uint8_t stop);
#endif
