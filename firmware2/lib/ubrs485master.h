#ifndef _UBRS485MASTER_H_
#define _UBRS485MASTER_H_

inline void rs485master_rx(void);
inline void rs485master_tx(void);
inline void rs485master_edge(void);
inline void rs485master_timer(void);

void rs485master_init(void);
#endif
