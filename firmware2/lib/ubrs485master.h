#ifndef _UBRS485MASTER_H_
#define _UBRS485MASTER_H_

inline void rs485master_rx(void);
inline void rs485master_tx(void);
inline void rs485master_edge(void);
inline void rs485master_timer(void);
inline void rs485master_txend(void);


void rs485master_init(void);
void rs485master_start(uint8_t start, uint8_t * data,
                        uint8_t len, uint8_t stop);
void rs485master_runslot(void);
#endif
