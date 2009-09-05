#ifndef _UBRS485CLIENT_H_
#define _UBRS485CLIENT_H_

inline void rs485client_rx(void);
inline void rs485client_tx(void);
inline void rs485client_edge(void);
inline void rs485client_timer(void);
inline void rs485client_txend(void);
void rs485client_init(void);
#endif
