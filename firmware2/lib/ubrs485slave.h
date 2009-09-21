#ifndef _UBRS485SLAVE_H_
#define _UBRS485SLAVE_H_

inline void rs485slave_rx(void);
inline void rs485slave_tx(void);
inline void rs485slave_edge(void);
inline void rs485slave_timer(void);
inline void rs485slave_txend(void);
void rs485slave_init(void);
inline void rs485slave_tick(void);
inline void rs485slave_process(void);

#endif
